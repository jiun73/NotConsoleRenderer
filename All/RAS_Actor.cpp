#include "pch.h"
#include "RAS_Actor.h"

size_t RAS::Manager::get_field_offset(FieldKey key, FieldID field)
{
	//assert((key & (1ull << field)) == 0); // verify that the key actually has the field

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
	return get_field_offset(key, (size_t(0) - 1));
}

void RAS::Manager::allocate_actor_data(Actor& actor)
{
	FieldKey copy = actor.key;

	size_t size = get_fields_size(actor.key);
	RawData data = new char[size];

	size_t i = 0;
	while (copy != 0)
	{
		if (copy & 1)
		{
			field_types.at(i)->construct(data + i);
			i += field_types.at(i)->size();
		}
		copy >>= 1;
	}
}

void RAS::Manager::register_actor(FieldKey key)
{
	Actor actor;
	actor.key = key;
	allocate_actor_data(actor);
}

void RAS::Manager::register_generator(const Generator& generator)
{
	generators.push_back(generator);
}

void RAS::Manager::set_start(Time time)
{
	start = time;
}

RAS::Time RAS::Manager::relative_time(Time time)
{
	return time - start;
}

void RAS::Manager::snapshot(Time time)
{
	for (auto& a : actors)
	{
		for (auto& f : a.timelines)
		{
			a.snapshot(time, get_field_offset(a.key, f.first), field_types.at(f.first)->type());
		}
	}
}

void RAS::Manager::register_system(const System& system)
{
	systems.push_back(system);
}
