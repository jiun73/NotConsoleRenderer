#pragma once
#include <type_traits>
#include <new>
#include <unordered_map>
#include <vector>
#include <array>
#include <tuple>
#include <map>
#include <cassert>
#include <iostream>
#include <bitset>
#include <algorithm>

#include "Singleton.h"

//roughly based on https://indiegamedev.net/2020/05/19/an-entity-component-system-with-data-locality-in-cpp/

//using std::launder;
using std::unordered_map;
using std::multimap;
using std::vector;
using std::array;
using std::pair;
using std::tuple;
using std::tuple_element_t;
using std::tuple_element_t;

const size_t MAX_COMPONENT = 32;
typedef unsigned char* ComponentData;
typedef size_t ComponentID;
typedef size_t EntityID;
typedef size_t SystemID;
typedef uint32_t ComponentBytes;

template<typename T>
class TypeId
{
private:
	static size_t current;

public:
	template<typename U>
	static const size_t id()
	{
		assert(current < MAX_COMPONENT);
		static const size_t count = current++;
		return count;
	}
};

template<class T> size_t TypeId<T>::current = 0;

inline ComponentBytes get_bytes_from_list(const vector<ComponentID>& list)
{
	ComponentBytes map = 0;
	for (auto& id : list)
	{
		map |= (1 << id);
	}
	return map;
}

inline vector<ComponentID> get_list_from_bytes(ComponentBytes bytes)
{
	vector<ComponentID> ret;
	for (size_t i = 0; i < 32; i++)
	{
		if (bytes & (1 << i))
		{
			ret.push_back(i);
		}
	}
	return ret;
}

struct ComponentX
{
	virtual void construct(ComponentData data) = 0;
	virtual void destruct(ComponentData data) = 0;
	virtual void move(ComponentData source, ComponentData destination) = 0;
	virtual size_t size() const = 0;
};

template<class C>
struct ComponentXFactory : public ComponentX
{
	void construct(ComponentData data) override
	{
		new (&data[0]) C();
	}

	void destruct(ComponentData data) override
	{
		C* location = reinterpret_cast<C*>(data);

		location->~C();
	}

	void move(ComponentData source, ComponentData destination) override
	{
		new (&destination[0]) C(std::move(*reinterpret_cast<C*>(source)));
	}

	size_t size() const override { return sizeof(C); }
};

struct SystemX
{
	virtual ComponentBytes key() = 0; //Key is used here to determine the system's required components
	virtual void create() = 0;
	virtual void update(const vector<ComponentData>& data, size_t size, const vector<size_t>& component_sizes) = 0;
};

template<typename T, typename... Reqs>
struct SystemXFactory : public SystemX
{
	T system;

	ComponentBytes _key;

	template<size_t I = 0>
	inline void collect_components(vector<ComponentID>& collector)
	{
		using tuple_type = tuple<Reqs...>;
		if constexpr (I < sizeof...(Reqs))
		{
			using element_t = tuple_element_t<I, tuple_type>;
			ComponentID id = TypeId<ComponentX>::id<element_t>();
			collector.push_back(id);
			collect_components<I + 1>(collector);
		}
	}

	void create() override
	{
		vector<ComponentID> list;
		collect_components(list);
		_key = get_bytes_from_list(list);
	}


	template<size_t I = 0, typename... Rs>
	inline void call_update(const vector<ComponentData>& data, const vector<size_t>& size, const size_t& index, Rs*... rest)
	{
		if constexpr (I < sizeof...(Reqs))
		{
			using type = tuple_element_t<I, tuple<Reqs...>>;
			call_update<I + 1>(data, size, index, rest..., reinterpret_cast<type*>(&data.at(I)[index * size.at(I)]));
		}
		else if constexpr (I == sizeof...(Reqs))
		{
			system.update(rest...);
		}
	}

	void update(const vector<ComponentData>& data, size_t size, const vector<size_t>& component_sizes) override
	{
		for (size_t i = 0; i < size; i++)
		{
			call_update<0>(data, component_sizes, i);
		}
	}

	ComponentBytes key() override
	{
		return _key;
	}

};

struct ArchetypeX
{
	size_t componentCount = 0;
	size_t entityCount = 0;
	unordered_map< ComponentID, ComponentData> data;
	unordered_map< ComponentID, size_t> data_size;
	unordered_map < EntityID, size_t> redirects;
	ComponentBytes key = 0;
};

class EntityManagerX
{
private:
	array<ComponentX*, MAX_COMPONENT> factories;
	unordered_map<ComponentBytes, ArchetypeX> archetypes;
	multimap<uint8_t, SystemX*> systems;
	EntityID entity_counter = 0;

	void add_archetype(ComponentBytes key)
	{
		archetypes.emplace(key, ArchetypeX());
		ArchetypeX& arch = archetypes.at(key);
		arch.key = key;
		std::cout << "Archetype " << std::bitset<32>(key) << " added" << std::endl;
	}

public:
	template<typename T>
	T* get_entity_component(EntityID eid, ComponentBytes key)
	{
		ComponentID cid = TypeId<ComponentX>::id<T>();
		ArchetypeX& arch = archetypes.at(key);
		size_t index = arch.redirects.at(eid);
		size_t size = arch.data_size.at(cid);
		ComponentData data = arch.data.at(cid);

		return reinterpret_cast<T*>(&data[index * size]);
	}

	template<typename T>
	void register_component_type()
	{
		size_t new_id = TypeId<ComponentX>::id<T>();
		factories.at(new_id) = new ComponentXFactory<T>();
		std::cout << "Component " << new_id << " added" << std::endl;
	}

	template<typename T, typename... Reqs>
	void register_system(uint8_t layer)
	{
		SystemX* new_system = new SystemXFactory<T, Reqs...>();
		new_system->create();
		systems.emplace(layer, new_system);
		std::cout << "System " << std::bitset<32>(new_system->key()) << " added" << std::endl;
	}

	pair<EntityID, ComponentBytes> add_entity(const vector<ComponentID>& list)
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
			ComponentX* factory = factories.at(i);

			size_t& size = archetype.data_size[id];
			size_t oldSize = size;
			size_t compSize = factory->size();

			if (size < archetype.entityCount) //is there is not enough size
			{
				if (size == 0) size = 2; else size *= 2;
				ComponentData data = new unsigned char[compSize * size]; //reallocate 

				std::cout << "Reallocating to " << size << " from " << oldSize << std::endl;

				for (size_t y = 0; y < oldSize; y++)
				{
					factory->move(&archetype.data[id][y * compSize], &data[y * compSize]); //move data to the new storage
					factory->destruct(&archetype.data[id][y * compSize]);
				}

				delete[](archetype.data[id]); //delete old data

				archetype.data[id] = data; //set the new data
			}

			factory->construct(&archetype.data[id][(archetype.entityCount - 1) * compSize]); //create the new component data
			archetype.redirects.emplace(new_id, archetype.entityCount - 1);
		}

		std::cout << "Entity " << new_id << " added" << std::endl;

		return { new_id ,byteform };
	}

	void update()
	{
		for (auto& pair : systems)
		{
			SystemX* system = pair.second;
			ComponentBytes sys_key = system->key();

			for (auto& arch : archetypes)
			{
				if ((arch.first & sys_key) == sys_key) //if system's component is a subset of archtype's
				{
					//std::cout << "Updating system " << std::bitset<32>(sys_key) << std::endl;
					ArchetypeX& archetype = arch.second;
					vector<ComponentData> alldata;
					vector<size_t> sizes;

					for (auto& component_type : archetype.data)
					{
						if (sys_key & (1 << component_type.first)) //component is in system
						{
							//std::cout << "Adding Component " << component_type.first << std::endl;
							alldata.push_back(component_type.second);
							sizes.push_back(factories.at(component_type.first)->size());
						}
					}

					system->update(alldata, archetype.entityCount, sizes);
				}
			}
		}
	}
};

typedef Singleton<EntityManagerX> EntX;

template<typename... Reqs>
class EntityX
{
private:
	EntityID id = 0;
	ComponentBytes key = 0;

	bool created = false;

public:
	EntityX() {}
	~EntityX() {}

	template<size_t I>
	inline void collect_components(vector<ComponentID>& collector)
	{
		using tuple_type = tuple<Reqs...>;
		if constexpr (I < sizeof...(Reqs))
		{
			using element_t = tuple_element_t<I, tuple_type>;
			ComponentID id = TypeId<ComponentX>::id<element_t>();
			collector.push_back(id);
			collect_components<I + 1>(collector);
		}
	}

	template<typename T>
	inline void set_arguments(const T& arg)
	{
		T* comp = EntX::get()->get_entity_component<T>(id, key);
		*comp = arg;
	}

	template<typename T, typename... Rs>
	inline void set_arguments(const T& arg, const Rs&... rest)
	{
		set_arguments(arg);
		set_arguments(rest...);
	}

	void create(const Reqs&... args)
	{
		vector<ComponentID> components;
		collect_components<0>(components);
		std::sort(components.begin(), components.end(), [](ComponentID a, ComponentID b)
			{
				return a > b;
			});
		created = true;

		auto pair = EntX::get()->add_entity(components);
		id = pair.first;
		key = pair.second;

		set_arguments(args...);
	}
};

template<typename T>
struct ComponentXAdder
{
	ComponentXAdder() { EntX::get()->register_component_type<T>(); }
	~ComponentXAdder() {}
};

template<size_t L, typename T, typename... Reqs>
struct SystemXAdder
{
	SystemXAdder() { EntX::get()->register_system<T, Reqs...>(L); }
	~SystemXAdder() {}
};

