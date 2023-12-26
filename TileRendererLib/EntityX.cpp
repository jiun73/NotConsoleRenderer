#include "EntityX.h"

inline void EntityManagerX::add_archetype(ComponentBytes key)
{
	archetypes.emplace(key, ArchetypeX());
	ArchetypeX& arch = archetypes.at(key);
	arch.key = key;
	std::cout << "Archetype " << std::bitset<32>(key) << " added" << std::endl;
}

bool EntityManagerX::entity_has_component(EntityID eid, ComponentID cid)
{
	size_t key = entities.at(eid);

	return (key & (1ull << cid));
}

ComponentData EntityManagerX::get_entity_component_data(EntityID eid, ComponentID cid)
{
	size_t key = entities.at(eid);

	ArchetypeX& arch = archetypes.at(key);
	size_t index = arch.redirects.at(eid);

	size_t size = factories.at(cid)->size();
	ComponentData data = arch.data.at(cid);

	return &data[index * size];
}

void EntityManagerX::remove_entity(EntityID eid)
{
	size_t key = entities.at(eid);
	ArchetypeX& archetype = archetypes.at(key);

	size_t oldIndex = archetype.redirects.at(eid);
	size_t latestIndex = archetype.entityCount - 1;
	size_t latestEntity = archetype.get_entity_at_id(latestIndex);

	archetype.redirects.erase(eid);

	if (oldIndex != latestIndex)
	{
		archetype.redirects.erase(latestEntity);
		archetype.redirects.emplace(latestEntity, oldIndex);
	}

	archetype.entityCount--;

	for (auto& pair : archetype.data)
	{
		ComponentData data = pair.second;
		ComponentX* factory = factories.at(pair.first);
		size_t size = factory->size();

		factory->destruct(&data[size * oldIndex]); //destroy the data we needed to destroy

		if (oldIndex != latestIndex) {
			factory->move(&data[size * latestIndex], &data[size * oldIndex]); //move the latest components to fill the slot we just freed
			factory->destruct(&data[size * latestIndex]);
		}

		//TODO maybe deallocate that memory we don't need anymore
	}

	entities.erase(eid);

	//std::cout << "Entity " << eid << " destroyed" << std::endl;
}

EntityID EntityManagerX::add_entity(const vector<ComponentID>& list)
{
	ComponentBytes byteform = get_bytes_from_list(list);

	int componentCount = list.size();

	if (!archetypes.count(byteform)) //if archetype doesn't exist, create it
	{
		add_archetype(byteform);
	}

	ArchetypeX& archetype = archetypes.at(byteform);
	archetype.entityCount++;
	size_t new_id = entity_counter++;

	for (size_t i = 0; i < componentCount; i++)
	{
		ComponentID id = list.at(i); //Resize the vector for the new data
		ComponentX* factory = factories.at(id);

		size_t& size = archetype.data_size[id];
		size_t oldSize = size;
		size_t compSize = factory->size();

		if (size < archetype.entityCount) //is there is not enough size
		{
			if (size == 0) size = 2; else size *= 2;
			ComponentData data = new unsigned char[compSize * size]; //reallocate 

			//std::cout << "Reallocating to " << size << " from " << oldSize << std::endl;

			for (size_t y = 0; y < oldSize; y++)
			{
				factory->move(&archetype.data[id][y * compSize], &data[y * compSize]); //move data to the new storage
				factory->destruct(&archetype.data[id][y * compSize]);
			}

			delete[](archetype.data[id]); //deallocate old data

			archetype.data[id] = data; //set the new data
		}

		factory->construct(&archetype.data[id][(archetype.entityCount - 1) * compSize]); //create the new component data
		archetype.redirects.emplace(new_id, archetype.entityCount - 1);
	}

	//std::cout << "Entity " << new_id << " added" << std::endl;

	entities.emplace(new_id, byteform);

	return new_id;
}

void EntityManagerX::add_callback(function<void(EntityID)> function, EntityID eid)
{
	callbacks.push_back({ eid, function });
}

void EntityManagerX::update()
{
	for (auto& pair : systems)
	{
		SystemX* system = pair.second;
		ComponentBytes sys_key = system->key();

		for (auto& arch : archetypes)
		{
			if ((arch.first & sys_key) == sys_key) //if system's component is a subset of archtype's
			{
				current_arch = arch.first;
				const ArchetypeX& archetype = arch.second;
				map<ComponentID, size_t> sizes;

				for (auto& component_type : archetype.data)
				{
					if (sys_key & (1 << component_type.first)) //component is in system
					{
						sizes.emplace(component_type.first, factories.at(component_type.first)->size());
					}
				}

				system->update(archetype, sizes, index);
			}
		}
	}

	for (auto& pair : systems)
	{
		pair.second->after_update();
	}

	do_callbacks();
}

void EntityManagerX::do_callbacks()
{
	for (auto& pair : callbacks)
	{
		pair.second(pair.first);
	}

	callbacks.clear();
}
