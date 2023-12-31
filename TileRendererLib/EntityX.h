#pragma once
#include <type_traits>
#include <new>
#include <unordered_map>
#include <map>
#include <vector>
#include <array>
#include <tuple>
#include <map>
#include <cassert>
#include <iostream>
#include <bitset>
#include <algorithm>
#include <functional>

#include "Singleton.h"

//roughly based on https://indiegamedev.net/2020/05/19/an-entity-component-system-with-data-locality-in-cpp/

/*
* En gros c'est un truc compliqué pour mieux organiser mon jeu
* Si tu veut un exemple de comment l'utiliser, tu peut voir mon jeu, mais c'est pas vraiment nécessaire
* mais si tu sait comment l'utiliser ca rend la vie plus facile
* 
* Ca créé des 'entités' qui ont des données (les 'components')
* Selon les components qu'un entité a, il y a différents systèmes qui vont transformer ces données
* Par exemple, si un entité a le component 'Physics' et 'Position'
* Peut être qu'un systeme pourrait calculer la gravité et changer la position en conséquence
* En tout cas, c'est utile, mais peut être trop complexe pour l'instant
*/

using std::launder;
using std::unordered_map;
using std::map;
using std::multimap;
using std::vector;
using std::array;
using std::pair;
using std::tuple;
using std::tuple_element_t;
using std::function;

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
		C* location = launder(reinterpret_cast<C*>(data));

		location->~C();
	}

	void move(ComponentData source, ComponentData destination) override
	{
		new (&destination[0]) C(std::move(*reinterpret_cast<C*>(source)));
	}

	size_t size() const override { return sizeof(C); }
};

struct ArchetypeX
{
	size_t entityCount = 0;
	unordered_map< ComponentID, ComponentData> data;
	unordered_map< ComponentID, size_t> data_size;
	unordered_map < EntityID, size_t> redirects;
	unordered_map < size_t, EntityID> redirects_reverse;
	ComponentBytes key = 0;

	void add_redirect(EntityID eid, size_t index)
	{
		redirects.emplace(eid, index);
		redirects_reverse.emplace(index, eid);
	}

	void remove_redirect_entity(EntityID eid)
	{
		size_t index = redirects.at(eid);
		redirects.erase(eid);
		redirects_reverse.erase(index);
	}

	EntityID get_entity_at_id(size_t index) 
	{
		return redirects_reverse.at(index);

		for (auto& pair : redirects) //we find the entity that is at the end of the list, not sure if there is a better way to do this
		{
			if (pair.second == index) //what we're going to do is move the latest entity to fill the spot we just freed
			{
				return pair.first;
			}
		}

		assert(0);

		return 0;
	}
};

struct SystemX
{
	virtual ComponentBytes key() = 0; //Key is used here to determine the system's required components
	virtual void create() = 0;
	virtual void update(const ArchetypeX& arch, const map<ComponentID, size_t>& sizes, size_t& index) = 0;
	virtual void after_update() = 0;
	virtual char* get_this() = 0;
};

template<typename T, typename... Reqs>
struct SystemXFactory : public SystemX
{
	T system;

	ComponentBytes _key = 0;

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
	inline void call_update(const ArchetypeX& arch, const map<ComponentID, size_t>& sizes, const size_t& index, Rs*... rest)
	{
		if constexpr (I < sizeof...(Reqs))
		{
			using type = tuple_element_t<I, tuple<Reqs...>>;
			ComponentID cid = TypeId<ComponentX>::id<type>();
			call_update<I + 1>(
				arch, sizes, index, rest..., 
				reinterpret_cast<type*>(&arch.data.at(cid)[index * sizes.at(cid)]));
		}
		else if constexpr (I == sizeof...(Reqs))
		{
			system.update(rest...);
		}
	}

	void update(const ArchetypeX& arch, const map<ComponentID, size_t>& sizes,size_t& index) override
	{
		for (index = 0; index < arch.entityCount; index++)
		{
			call_update<0>(arch, sizes, index);
		}
	}

	void after_update() override {}

	ComponentBytes key() override
	{
		return _key;
	}

	char* get_this() override { return reinterpret_cast<char*>( & system); }
};

template<typename T, typename... Reqs>
class SystemXManagerFactory : public SystemX
{
	T system;
	tuple<vector<Reqs*>...> collected_data;
	vector<EntityID> ids;

	char* get_this() override { return reinterpret_cast<char*>(&system); }

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

	ComponentBytes _key = 0;

	ComponentBytes key() override
	{
		return _key;
	}

	template<size_t I>
	void collect_data(const ArchetypeX& arch, const map<ComponentID, size_t>& sizes, size_t& index)
	{
		using tuple_type = tuple<Reqs...>;
		if constexpr (I < sizeof...(Reqs))
		{
			using element_t = tuple_element_t<I, tuple_type>;
			ComponentID id = TypeId<ComponentX>::id<element_t>();

			std::get<I>(collected_data).push_back(reinterpret_cast<element_t*>(&arch.data.at(id)[sizes.at(id) * index]));

			collect_data<I + 1>(arch, sizes, index);
		}
	}

	void update(const ArchetypeX& arch, const map<ComponentID, size_t>& sizes, size_t& index) override
	{
		for (index = 0; index < arch.entityCount; index++)
		{
			collect_data<0>(arch, sizes, index);
			ids.push_back(arch.redirects_reverse.at(index));
		}
	}

	template<size_t I, typename... Rs>
	void unfold_tuple(vector<Rs*>&... args)
	{
		if constexpr (I < sizeof...(Reqs))
		{
			unfold_tuple<I + 1>(args..., std::get<I>(collected_data));
		}
		else if constexpr (I == sizeof...(Reqs))
		{
			system.update(args..., ids);
		}
	}

	template<size_t I = 0>
	void clear_tuple() 
	{
		if constexpr (I < sizeof...(Reqs))
		{
			std::get<I>(collected_data).clear();
			clear_tuple<I + 1>();
		}
	}

	void after_update() override 
	{
		unfold_tuple<0>();
		clear_tuple();
		ids.clear();
	}
};

class EntityManagerX
{
private:
	array<ComponentX*, MAX_COMPONENT> factories;
	unordered_map<ComponentBytes, ArchetypeX> archetypes;
	multimap<uint8_t, SystemX*> systems;
	map<SystemID, SystemX*> systems_by_id;

	unordered_map<EntityID, ComponentBytes> entities;
	EntityID entity_counter = 0;

	vector<pair<EntityID, function<void(EntityID)>>> callbacks;

	ComponentBytes current_arch;
	size_t index;

	void add_archetype(ComponentBytes key);
	void add_system(uint8_t layer, SystemX* system, SystemID id);

public:
	bool has_entity(EntityID eid) { return entities.count(eid); }
	bool entity_has_component(EntityID eid, ComponentID cid);
	template<typename T>
	bool entity_has_component(EntityID eid) { return entity_has_component(eid, TypeId<ComponentID>::id<T>()); }

	ComponentData get_entity_component_data(EntityID eid, ComponentID cid);

	template<typename T> T* get_entity_component(EntityID eid);

	template<typename T>					void register_component_type();
	template<typename T, typename... Reqs>	void register_system(uint8_t layer);
	template<typename T, typename... Reqs>	void register_system_manager(uint8_t layer);
	template<typename T>					T* get_system();

	void remove_entity(EntityID eid);
	EntityID add_entity(const vector<ComponentID>& list);

	void add_callback(function<void(EntityID)> function, EntityID eid);
	void add_callback_current(function<void(EntityID)> function)
	{
		ArchetypeX& arch = archetypes.at(current_arch);
		EntityID entity = arch.get_entity_at_id(index);
		add_callback(function, entity);
	}
	void destroy_this()
	{
		add_callback_current([](EntityID eid)
			{
				Singleton<EntityManagerX>::get()->remove_entity(eid);
			});
	}
	void destroy_this(EntityID eid)
	{
		add_callback([](EntityID eid)
			{
				if(Singleton<EntityManagerX>::get()->has_entity(eid))
					Singleton<EntityManagerX>::get()->remove_entity(eid);
			}, eid);
	}

	void do_callbacks();

	void update();
};

typedef Singleton<EntityManagerX> EntX;

template<typename... Reqs>
class EntityX
{
private:
	EntityID id = 0;

	bool created = false;

public:
	EntityX() {}
	~EntityX() {}

	EntityID get_id() { return id; }

	template<typename T>
	T* component() { return EntX::get()->get_entity_component<T>(id); }

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
		T* comp = EntX::get()->get_entity_component<T>(id);
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

		id = EntX::get()->add_entity(components);

		set_arguments(args...);
	}

	void destroy()
	{
		EntX::get()->remove_entity(id);
		created = false;
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

template<size_t L, typename T, typename... Reqs>
struct SystemXManagerAdder
{
	SystemXManagerAdder() { EntX::get()->register_system_manager<T, Reqs...>(L); }
	~SystemXManagerAdder() {}
};

//Definitions

template<typename T>
inline T* EntityManagerX::get_entity_component(EntityID eid)
{
	return reinterpret_cast<T*>(get_entity_component_data(eid, TypeId<ComponentX>::id<T>()));
}

template<typename T>
inline void EntityManagerX::register_component_type()
{
	size_t new_id = TypeId<ComponentX>::id<T>();
	factories.at(new_id) = new ComponentXFactory<T>();
	std::cout << "Component " << new_id << " added" << std::endl;
}

template<typename T, typename ...Reqs>
inline void EntityManagerX::register_system(uint8_t layer)
{
	SystemX* new_system = new SystemXFactory<T, Reqs...>();
	add_system(layer, new_system, TypeId<SystemX>::id<T>());
}

template<typename T, typename ...Reqs>
inline void EntityManagerX::register_system_manager(uint8_t layer)
{
	SystemX* new_system = new SystemXManagerFactory<T, Reqs...>();
	add_system(layer, new_system, TypeId<SystemX>::id<T>());
}

template<typename T>
inline T* EntityManagerX::get_system()
{
	SystemX* system = systems_by_id.at(TypeId<SystemX>::id<T>());

	return reinterpret_cast<T*>(system->get_this());
}

