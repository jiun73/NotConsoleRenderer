#pragma once
#include <map>
#include <array>
#include <vector>
#include <functional>

namespace RAS {
	typedef size_t Time;
	typedef size_t GeneratorID;
	typedef size_t ActorID;
	typedef size_t FieldID;
	typedef char* RawData;
	typedef uint64_t FieldKey;

	using std::map;
	using std::array;
	using std::vector;
	using std::function;


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
		const type_info& type() override
		{
			return typeid(T);
		}

		void construct(RawData data) override
		{
			new (&data[0]) T();
		}

		void destruct(RawData data) override
		{
			T* location = launder(reinterpret_cast<T*>(data));

			location->~T();
		}

		void move(RawData source, RawData destination) override
		{
			new (&destination[0]) T(std::move(*reinterpret_cast<T*>(source)));
		}

		size_t size() const override { return sizeof(T); }
	};
	 
	struct Manager;

	//A function that changes data according to the time
	struct Modifier 
	{
		virtual bool is_reversible() = 0;  
		virtual size_t reverse_type() = 0;
		virtual vector<double> reverse_params() = 0;
		virtual void apply(Time time, RawData data, type_info type) = 0;
	};

	template<typename T>
	struct ModifierType : public Modifier //Standard, non reversible modifiers
	{
		function<void(T&)> mod_func;

		bool is_reversible() { return false; };
		size_t reverse_type() { return 0; };
		vector<double> reverse_params() { return {} };

		void apply(Time time, RawData data, type_info type) override;
	};

	template<typename T, size_t S>
	struct ModifierPureType : public Modifier //Modifers that use pure math functions that are reversible, allowing them to be used in things like collision detection
	{
		function<void(T&, const array <T, S>&)> mod_func;
		array<T, S> params;
		size_t type = 0;

		bool is_reversible() { return true; };
		size_t reverse_type() { return type; };
		vector<double> reverse_params() { return params; };

		void apply(Time time, RawData data, type_info type) override;
	};

	//Sequence of modifiers, describing the evolution in time of a data field according to external input
	struct Event
	{
		Manager* manager;
		GeneratorID generator;
		map<Time, Modifier> modifiers;

		const Modifier& modifier_at(Time time, RawData data, type_info type) const;
	};

	//Generates Events of a certain type 
	struct Generator 
	{
		function<Event(Manager*)> generate;
	};

	//Set of events for an Actor
	struct Timeline
	{
		map<Time, Event> events;

		const Event& event_at(Time time, RawData data, type_info type) const;
	};

	struct Actor
	{
		Timeline timeline;
		RawData data;
		FieldKey key;

		void snapshot(Time time, size_t field_offset, type_info type);
	};

	//Handles semi-deterministic events, like collision detection
	struct System
	{
		function<void(Time, vector<Actor>&)> update;
	};

	//Interface for the developper
	struct Manager
	{
		map<FieldKey, DataTypeFactory*> field_types;
		vector<Actor> actors;
		vector<System> systems;
		vector<Generator> generators;

		void register_actor();
		void register_system();
		void register_generator();

		void set_start(Time time);
		void snapshot(Time time);
		void trigger_systems(Time time);
		void regenerate_from(Time time);
		void add_event(ActorID actor, GeneratorID generator);
	};
}