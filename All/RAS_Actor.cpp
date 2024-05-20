#include "pch.h"
#include "RAS_Actor.h"

size_t RAS::Manager::get_field_offset(FieldKey key, FieldID field, bool check)
{
	if(check)
		assert((key & (1ull << field)) > 0); // verify that the key actually has the field

	FieldKey copy = key;
	size_t i = 0;
	size_t size = 0;
	while (copy != 0)
	{
		if (copy & 1)
		{
			size += field_types.at(i)->size();
		}
		copy >>= 1;
		i++;

		if (i >= field) return size;
	}
	return size;
}

size_t RAS::Manager::get_fields_size(FieldKey key)
{
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

RAS::ActorID RAS::Manager::register_actor(FieldKey key)
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
	return actors.size() - 1;
}

RAS::GeneratorID RAS::Manager::register_generator(const Generator& generator)
{
	generators.push_back(generator);
	return generators.size() - 1;
}

void RAS::Manager::set_start()
{
	start = time_fetch();
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

void RAS::Manager::snapshot(Time time)
{
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

void RAS::Manager::add_event(Time time, ActorID actor, GeneratorID generator)
{
	add_event(time, actor, generator, generators.at(generator).field);
}

void RAS::Manager::add_event(Time time, ActorID actor, GeneratorID generator, FieldID field)
{
	const Generator& gen = generators.at(generator);
	Event e = gen.generate(time, this, field, actor);
	e.start_time = time;
	actors.at(actor).timelines.at(field).events.emplace(time, e);
	trigger_systems(time);
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
}

void RAS::Timeline::snapshot(Time time, RawData data, const type_info& type)
{
	event_at(time).snapshot(time, data, type);
}

RAS::Modifier* RAS::Event::modifier_at(Time time) const
{
	auto it = modifiers.lower_bound(time);

	if (it != modifiers.begin())
	{
		return (--it)->second;
	}
}

void RAS::Event::snapshot(Time time, RawData data, const type_info& type) const
{
	modifier_at(time)->apply(time - start_time, data, type);
}
