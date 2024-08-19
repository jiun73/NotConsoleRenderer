#pragma once
#include <map>
#include <array>
#include <vector>
#include <memory>
#include <functional>

#include "Lexer.h"

/*
* Rollback Actor System
* - Entity system optimised for precise rollback netcode
*/

namespace RAS {
	typedef uint32_t Time;
	typedef size_t GeneratorID;
	typedef size_t ActorID;
	typedef size_t FieldID;
	typedef uint16_t TriggerID;

	typedef size_t GeneratorAlias;
	typedef size_t ActorAlias;
	typedef size_t FieldAlias;
	typedef size_t TriggerAlias;

	typedef char* RawData;
	typedef uint64_t FieldKey;

	using std::map;
	using std::array;
	using std::vector;
	using std::function;
	using std::launder;

	struct DataTypeFactory
	{
		virtual void construct(RawData data) = 0;
		virtual void destruct(RawData data) = 0;
		virtual void move(RawData source, RawData destination) = 0;
		virtual size_t size() const = 0;
		virtual const type_info& type() = 0;
	};

	template<typename T>
	class DataType : public DataTypeFactory
	{
		const type_info& type() override { return typeid(T); }
		void construct(RawData data) override { new (&data[0]) T(); }
		void destruct(RawData data) override { T* location = launder(reinterpret_cast<T*>(data)); location->~T(); }
		void move(RawData source, RawData destination) override { new (&destination[0]) T(std::move(*reinterpret_cast<T*>(source))); }
		size_t size() const override { return sizeof(T); }
	};

	struct Manager;

	//A function that changes data according to the time
	struct Modifier
	{
		virtual bool is_reversible() const = 0;
		virtual size_t reverse_type() const = 0;
		virtual vector<double> reverse_params() const = 0;
		virtual void apply(Time time, RawData data, const type_info& type) const = 0;
	};

	template<typename T>
	struct ModifierType : public Modifier //Standard, non reversible modifiers
	{
		function<void(Time, T&)> mod_func;

		bool is_reversible() const { return false; };
		size_t reverse_type() const { return 0; };
		vector<double> reverse_params() const { return {}; };

		void apply(Time time, RawData data, const type_info& type) const override
		{
			assert(typeid(T) == type);
			mod_func(time, *(T*)(data));
		}
	};

	template<typename T, size_t S>
	struct ModifierPureType : public Modifier //Modifers that use pure math functions that are reversible, allowing them to be used in things like collision detection
	{
		function<void(Time, T&, const array <double, S>&)> mod_func;
		array<double, S> params;
		size_t type = 0;

		bool is_reversible() const { return true; };
		size_t reverse_type() const { return type; };
		vector<double> reverse_params() const
		{
			vector<double> ret;
			for (auto l : params)
				ret.push_back(l);
			return ret;
		};

		void apply(Time time, RawData data, const type_info& type) const override
		{
			assert(typeid(T) == type);
			mod_func(time, *(T*)(data), params);
		}
	};

	//Sequence of modifiers, describing the evolution in time of a data field according to external input
	struct Event
	{
		Manager* manager;
		GeneratorID generator;
		bool regenerate = true;
		map<Time, Modifier*> modifiers;
		Time start_time = 0;
		vector<double> extra = {};

		map<Time, Modifier*>::iterator begin_from_time(Time time)
		{
			map<RAS::Time, Modifier*>::iterator event = modifiers.lower_bound(time);
			if (event != modifiers.begin()) { event--; }
			return event;
		}

		template <typename T>
		Event& add_modifier(Time time, function<void(Time, T&)> function)
		{
			ModifierType<T>* mod = new ModifierType<T>();
			mod->mod_func = function;
			modifiers.emplace(time, mod);
			return *this;
		}

		template <typename T, size_t I>
		Event& add_modifier(Time time, function<size_t(Time, T&, const array <double, I>&)> func, array<double, I> params)
		{
			ModifierPureType<T, I>* mod = new ModifierPureType<T, I>();
			mod->mod_func = func;
			mod->params = params;
			T temp;
			mod->type = func(-1, temp, params);
			modifiers.emplace(time, mod);
			return *this;
		}

		Modifier* modifier_at(Time time) const;
		pair<Time, Modifier*> modifier_at_pair(Time time) const;
		Modifier* modifier_at_absolute(Time time) const;
		pair<Time, Modifier*> modifier_at_absolute_pair(Time time) const;
		const std::pair<const size_t, Modifier*>& pair_at(Time time) const;
		void snapshot(Time time, RawData data, const type_info& type) const;
		bool is_gen(GeneratorAlias alias) const;
	};

#define GENERATOR_ARGS RAS::Time time, RAS::Manager* manager, RAS::FieldID generator_field, RAS::ActorID actor, const std::vector<double>& extra
#define GENERATOR_ARGS_PASS time, manager, generator_field, actor, extra

	//Generates Events of a certain type 
	struct Generator
	{
		FieldID field = -1;
		function<Event(Time, Manager*, FieldID, ActorID, const vector<double>&)> generate;

		Generator() {}
		Generator(function<Event(Time, Manager*, FieldID, ActorID, const vector<double>&)> generate) : generate(generate) {}
		Generator(function<Event(Time, Manager*, FieldID, ActorID, const vector<double>&)> generate, FieldID field) : generate(generate), field(field) {}
		~Generator() {}
	};

	//Set of events for an Actor
	struct Timeline
	{
		map<Time, Event> events;

		const Event& event_at(Time time) const;
		void snapshot(Time time, RawData data, const type_info& type);
		map<Time, Event>::iterator begin_from_time(Time time)
		{
			map<RAS::Time, RAS::Event>::iterator event = events.lower_bound(time);
			if (event != events.begin()) { event--; }
			return event;
		}

		Timeline() { }
		~Timeline() { }
	};

	struct Actor
	{
		map<FieldID, Timeline> timelines;
		RawData data;
		FieldKey key;

		void snapshot(Time time, FieldID field, RawData field_data, const type_info& type);
	};

	//Handles semi-deterministic events, like collision detection
	struct System
	{
		//function<void(Manager*, Time, vector<Actor>&)> update;

		virtual void update(Manager* manager, Time time, vector<Actor>& actors) = 0;
		virtual void on_snap(Manager* manager, Time time, vector<Actor>& actors) = 0;
	};

	template<typename T>
	struct SystemType : public System
	{
		T system;

		void on_snap(Manager* manager, Time time, vector<Actor>& actors) override
		{
			system.on_snap(manager, time, actors);
		}

		void update(Manager* manager, Time time, vector<Actor>& actors) override
		{
			system.update(manager, time, actors);
		}
	};

	struct TriggerArg
	{
		RAS::ActorAlias actor;
		RAS::GeneratorAlias gen;
		RAS::FieldAlias field;
		vector<double> extra;

		TriggerArg(RAS::ActorAlias actor,RAS::GeneratorAlias gen,RAS::FieldAlias field,vector<double> extra) : actor(actor), gen(gen), field(field), extra(extra) {}
		~TriggerArg() {}
	};

	//Maps a code to a set of events, allowing users to communicate via network with very few data
	struct Trigger 
	{
		vector<TriggerArg> events;
		int userid = -1; //change events added depending on the user that is triggering it

		Trigger(vector<TriggerArg> events) : events(events) {}
		~Trigger() {}

		void trigger(RAS::Time time, Manager* man, int userid, size_t id_diff);
 	};

	//Interface for the developper
	struct Manager
	{
		enum LexerEnum
		{
			FIELD,
			ACTOR,
			GEN,
			TRIGGER
		};

		Manager()
		{
			lexer.add_lexer(FIELD);
			lexer.add_lexer(ACTOR);
			lexer.add_lexer(GEN);
			lexer.add_lexer(TRIGGER);
		}
		~Manager() {}

		Lexer lexer;
		Time start = 0;
		function<Time()> time_fetch;
		function<void(Time, TriggerAlias)> send_func;
		vector<DataTypeFactory*> field_types;
		vector<Actor> actors;
		vector<System*> systems;
		vector<Generator> generators;
		map<int, map<TriggerID, Trigger>> triggers;
		int userid = -1;

		size_t get_field_offset(FieldKey key, FieldID field, bool check = true);
		size_t get_fields_size(FieldKey key);
		void allocate_actor_data(Actor& actor);
		RawData get_actor_field(const Actor& actor, FieldID field);
		bool has_field(FieldKey key, FieldID field);

		template<typename T>
		FieldID register_field(FieldAlias alias)
		{
			field_types.push_back(new DataType<T>());
			lexer.add_alias(FIELD, alias, field_types.size() - 1);
			return field_types.size() - 1;
		}
		ActorID register_actor(FieldKey key, ActorAlias alias);

		template<typename T>
		void register_system()
		{
			systems.push_back(new SystemType<T>());
		}

		GeneratorID register_generator(const Generator& generator, GeneratorAlias alias);
		TriggerID register_trigger(const Trigger& trigger, TriggerAlias alias, int user_specific = -1) 
		{
			triggers[user_specific].emplace(alias, trigger);
			lexer.add_alias(TRIGGER, alias, alias);
			return alias;
		}

		void set_start();
		void set_net(int userid, function<void(Time, TriggerID)> send_func) { this->userid = userid; this->send_func = send_func; }
		void set_start(Time time);
		void set_time_fetcher(function<Time()> func);
		Time relative_time(Time time);
		Time now();
		RawData snapshot_actor(Time time, ActorID actor, FieldID field, const std::type_info& type);
		void snapshot(Time time);
		void snapshot_now();
		void trigger_systems(Time time);
		void regenerate_from(Time time, bool delete_sys);
		void add_event_internal(Time time, ActorID actor, GeneratorID generator);
		void add_event_internal(Time time, ActorID actor, GeneratorID generator, FieldID field, bool system_generated = false, const vector<double>& extra = {});

		void add_event(Time time, ActorAlias actor, GeneratorAlias generator, FieldAlias field, bool system_generated = false);
		void add_event_extra(Time time, ActorAlias actor, GeneratorAlias generator, FieldAlias field, const vector<double>& extra, bool system_generated = false);

		const RAS::Event& get_event_at_internal(Time time, ActorID actor, FieldID field);
		const RAS::Event& get_event_at(Time time, ActorAlias actor, FieldAlias field) { return get_event_at_internal(time, get_actor(actor), get_field(field)); }

		Timeline& get_field_timeline(ActorAlias actor, FieldAlias field) 
		{
			return actors.at(actor).timelines.at(get_field(field));
		}

		void trigger(Time time, TriggerAlias alias, int userid)
		{
			TriggerID id = lexer.get_alias(TRIGGER, alias);

			if (triggers[userid].count(id))
			{
				triggers[userid].at(id).trigger(time, this, userid, 0);
				return;
			}
			else if (triggers[-1].count(id))
			{
				triggers[-1].at(id).trigger(time, this, userid, 0);
				return;
			}
			else
				assert(false);
		}

		void trigger_now(TriggerAlias alias) 
		{
			Time t = now();
			trigger(t, alias, userid);
			send_func(t, alias);
		}

		template<typename T>
		T& current_actor_field_internal(ActorID actor, FieldID field)
		{
			Actor& act = actors.at(actor);
			assert(field_types.at(field)->type() == typeid(T));
			return *(T*)(get_actor_field(act, field));
		}

		template<typename T>
		T& current_actor_field(ActorAlias actor, FieldAlias field)
		{
			return current_actor_field_internal<T>(lexer.get_alias(ACTOR, actor), lexer.get_alias(FIELD, field));
		}

		template<typename T>
		T& actor_field_at_internal(Time time, ActorID actor, FieldID field)
		{
			return *(T*)(snapshot_actor(time, actor, field, typeid(T)));
		}

		template<typename T>
		T& actorid_field_at(Time time, ActorID actor, FieldAlias field)
		{
			return actor_field_at_internal<T>(time, actor, lexer.get_alias(FIELD, field));
		}

		template<typename T>
		T& actor_field_at(Time time, ActorAlias actor, FieldAlias field)
		{
			return actor_field_at_internal<T>(time, lexer.get_alias(ACTOR, actor), lexer.get_alias(FIELD, field));
		}

		FieldID get_field(FieldAlias alias)
		{
			return lexer.get_alias(FIELD, alias);
		}

		ActorID get_actor(ActorAlias alias)
		{
			return lexer.get_alias(ACTOR, alias);
		}

		GeneratorID get_gen(GeneratorAlias alias)
		{
			return lexer.get_alias(GEN, alias);
		}

		//Modifier* make_modifier();
		//Event make_event();
	};
}