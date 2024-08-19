#include "pch.h"
#include "RAS.h"

size_t RAS::Manager::get_field_offset(FieldKey key, FieldID field, bool check)
{
	if (check)
		assert(has_field(key, field)); // verify that the key actually has the field

	FieldKey copy = key;
	size_t i = 0;
	size_t size = 0;
	while (copy != 0)
	{
		if (i >= field) return size;

		if (copy & 1)
		{
			size += field_types.at(i)->size();
		}
		copy >>= 1;
		i++;


	}
	return size;
}

size_t RAS::Manager::get_fields_size(FieldKey key)
{
	int a = 0;
	return get_field_offset(key, (size_t(0) - 1), false);
}

void RAS::Manager::allocate_actor_data(Actor& actor)
{
	FieldKey copy = actor.key;

	size_t size = get_fields_size(actor.key);
	RawData data = new char[size];

	size_t i = 0;
	size_t off = 0;
	while (copy != 0)
	{
		if (copy & 1)
		{
			field_types.at(i)->construct(data + off);
			off += field_types.at(i)->size();
		}
		copy >>= 1;
		i++;
	}

	actor.data = data;
}

RAS::RawData RAS::Manager::get_actor_field(const Actor& actor, FieldID field)
{
	return actor.data + get_field_offset(actor.key, field);
}

bool RAS::Manager::has_field(FieldKey key, FieldID field)
{
	return (key & (1ull << field)) > 0;
}

RAS::ActorID RAS::Manager::register_actor(FieldKey key, ActorAlias alias)
{
	Actor actor;
	actor.key = key;
	allocate_actor_data(actor);

	FieldKey copy = key;

	size_t i = 0;
	while (copy != 0)
	{
		if (copy & 1)
		{
			actor.timelines.emplace(i, Timeline());
		}
		copy >>= 1;
		i++;
	}

	actors.push_back(actor);
	lexer.add_alias(ACTOR, alias, actors.size() - 1);
	return actors.size() - 1;
}

RAS::GeneratorID RAS::Manager::register_generator(const Generator& generator, GeneratorAlias alias)
{
	generators.push_back(generator);
	RAS::GeneratorID gen_id = generators.size() - 1;
	lexer.add_alias(GEN, alias, gen_id);
	return gen_id;
}

void RAS::Manager::set_start()
{
	start = time_fetch();
}

void RAS::Manager::set_start(Time time)
{
	start = time;
}

void RAS::Manager::set_time_fetcher(function<Time()> func)
{
	time_fetch = func;
}

RAS::Time RAS::Manager::relative_time(Time time)
{
	return time - start;
}

RAS::Time RAS::Manager::now()
{
	return relative_time(time_fetch());
}

RAS::RawData RAS::Manager::snapshot_actor(Time time, ActorID actor, FieldID field, const std::type_info& type)
{
	for (auto& s : systems)
	{
		s->on_snap(this, time, actors);
	}

	Actor& act = actors.at(actor);
	RawData data = get_actor_field(act, field);
	act.timelines.at(field).snapshot(time, data, type);
	return data;
}

void RAS::Manager::snapshot(Time time)
{
	for (auto& s : systems)
	{
		s->on_snap(this, time, actors);
	}

	for (auto& a : actors)
	{
		for (auto& f : a.timelines)
		{
			a.snapshot(time, f.first, get_actor_field(a, f.first), field_types.at(f.first)->type());
		}
	}
}

void RAS::Manager::snapshot_now()
{
	snapshot(now());
}

void RAS::Manager::trigger_systems(Time time)
{
	for (auto& s : systems)
	{
		s->update(this, time, actors);
	}
}

void RAS::Manager::regenerate_from(Time time, bool delete_sys = true)
{
	ActorID actor = 0;
	for (auto& a : actors)
	{
		for (auto& t : a.timelines)
		{
			auto it = t.second.events.lower_bound(time);

			while (it != t.second.events.end())
			{
				if (delete_sys && !it->second.regenerate)
				{
					it = t.second.events.erase(it);
					continue;
				}
				else
				{
					GeneratorID genid = it->second.generator;
					std::vector<double> extra = it->second.extra;
					Generator& gen = generators.at(genid);
					it->second = gen.generate(it->first, this, t.first, actor, extra);
					it->second.extra = extra;
					it->second.start_time = it->first;
					it->second.generator = genid;
					it->second.manager = this;

				}

				it++;
			}
		}
		actor++;
	}
}

void RAS::Manager::add_event_internal(Time time, ActorID actor, GeneratorID generator)
{
	add_event_internal(time, actor, generator, generators.at(generator).field);
}

void RAS::Manager::add_event_internal(Time time, ActorID actor, GeneratorID generator, FieldID field, bool system_event, const vector <double>& extra)
{
	const Generator& gen = generators.at(generator);
	Event e = gen.generate(time, this, field, actor, extra);
	e.manager = this;
	e.start_time = time;
	e.generator = generator;
	e.regenerate = !system_event;
	e.extra = extra;
	actors.at(actor).timelines.at(field).events.emplace(time, e);
	if (system_event)
	{
		//std::cout << "new system event " << time << std::endl;
		regenerate_from(time + 1, false);
	}
	else {
		//std::cout << "new real event " << time << std::endl;
		regenerate_from(time);
		trigger_systems(time);
	}
}

void RAS::Manager::add_event(Time time, ActorAlias actor, GeneratorAlias generator, FieldAlias field, bool system_generated)
{
	add_event_internal(time, lexer.get_alias(ACTOR, actor), lexer.get_alias(GEN, generator), lexer.get_alias(FIELD, field), system_generated);
}

void RAS::Manager::add_event_extra(Time time, ActorAlias actor, GeneratorAlias generator, FieldAlias field, const vector<double>& extra, bool system_generated)
{
	add_event_internal(time, lexer.get_alias(ACTOR, actor), lexer.get_alias(GEN, generator), lexer.get_alias(FIELD, field), system_generated, extra);
}

const RAS::Event& RAS::Manager::get_event_at_internal(Time time, ActorID actor, FieldID field)
{
	return actors.at(actor).timelines.at(field).event_at(time);
}

void RAS::Actor::snapshot(Time time, FieldID field, RawData field_data, const type_info& type)
{
	timelines.at(field).snapshot(time, field_data, type);
}

const RAS::Event& RAS::Timeline::event_at(Time time) const
{
	auto it = events.lower_bound(time);

	if (it != events.begin())
	{
		return (--it)->second;
	}

	return it->second;
}

void RAS::Timeline::snapshot(Time time, RawData data, const type_info& type)
{
	if (events.size() == 0) return;
	event_at(time).snapshot(time, data, type);
}


RAS::Modifier* RAS::Event::modifier_at(Time time) const
{
	return modifier_at_pair(time).second;
}

pair<RAS::Time, RAS::Modifier*> RAS::Event::modifier_at_pair(Time time) const
{
	auto it = modifiers.lower_bound(time);

	if (it != modifiers.begin())
	{
		return *(--it);
	}

	return *it;
}

RAS::Modifier* RAS::Event::modifier_at_absolute(Time time) const
{
	return modifier_at(time - start_time);
}

pair<RAS::Time, RAS::Modifier*> RAS::Event::modifier_at_absolute_pair(Time time) const
{
	return modifier_at_pair(time - start_time);
}

const std::pair<const size_t, RAS::Modifier*>& RAS::Event::pair_at(Time time) const
{
	auto it = modifiers.lower_bound(time);

	if (it != modifiers.begin())
	{
		return *(--it);
	}

	return *it;
}
bool RAS::Event::is_gen(GeneratorAlias alias) const
{
	return generator == manager->get_gen(alias);
}

void RAS::Event::snapshot(Time time, RawData data, const type_info& type) const
{
	auto pair = pair_at(time - start_time);
	pair.second->apply(time - start_time - pair.first, data, type);
}

void RAS::Trigger::trigger(RAS::Time time, Manager* man, int userid, size_t id_diff)
{
	for (auto& e : events)
	{
		man->add_event_extra(time, e.actor + id_diff, e.gen, e.field, e.extra);
	}
}
