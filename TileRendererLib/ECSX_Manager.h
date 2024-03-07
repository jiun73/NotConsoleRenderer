#pragma once
#include "ECSX_System.h"
#include <array>
#include <map>
#include <functional>
#include <iomanip>

namespace ECSX 
{
	using std::map;
	using std::multimap;
	using std::array;
	using std::pair;
	using std::function;

	class Manager
	{
		template<typename T>
		friend struct ComponentXAdder;
		template<size_t L, typename T, typename... Reqs>
		friend struct SystemXAdder;
		template<size_t L, typename T, typename... Reqs>
		friend struct SystemXManagerAdder;

	private:
		array<ComponentX*, MAX_COMPONENT> factories = {};
		unordered_map<ComponentBytes, ArchetypeX> archetypes;
		multimap<uint8_t, SystemX*> systems;
		map<SystemID, SystemX*> systems_by_id;

		unordered_map<EntityID, ComponentBytes> entities;
		multimap<EntityTag, EntityID> entity_tags;
		map< EntityID, EntityTag> entity_tags_reversed;
		EntityID entity_counter = 0;

		vector<pair<EntityID, function<void(Manager&, EntityID)>>> callbacks;

		ComponentBytes current_arch = 0;
		size_t index = 0;

		void add_archetype(ComponentBytes key);
		void add_system(uint8_t layer, SystemX* system, SystemID id);

		void remove_tag_pair(EntityTag tag, EntityID eid);

		void add_tag_pair(EntityTag tag, EntityID eid)
		{
			entity_tags.emplace(tag, eid);
			entity_tags_reversed.emplace(eid, tag);
		}


		bool entity_has_component(EntityID eid, ComponentID cid);
		ComponentData get_entity_component_data(EntityID eid, ComponentID cid);

		EntityID	add_entity(const vector<ComponentID>& list);
		EntityID	add_entity_tagged(const vector<ComponentID>& list, EntityTag tag);

		template<typename T>					void register_component_type();
		template<typename T, typename... Reqs>	void register_system(uint8_t layer);
		template<typename T, typename... Reqs>	void register_system_manager(uint8_t layer);

		void set_entity_component(EntityID id) {}

		template<typename T>
		void set_entity_component(EntityID id, const T& component)
		{
			T* comp = get_entity_component<T>(id);
			*comp = component;
		}

		template<typename T, typename... Rs>
		void set_entity_component(EntityID id, const T& component, const Rs&... rest)
		{
			set_entity_component(id, component);
			set_entity_component(id, rest...);
		}

		void do_callbacks();

	public:
		bool has_entity(EntityID eid) { return entities.count(eid); }
		template<typename T> bool entity_has_component(EntityID eid) { return entity_has_component(eid, TypeId<ComponentID>::id<T>()); }
		template<typename T> T* get_entity_component(EntityID eid);
		template<typename T> T* get_system();

		template<typename... Comp>
		EntityID	add_entity(const Comp&... components, EntityTag tag = 0)
		{
			vector<ComponentID> components_list;
			ComponentCollector<Comp...> collector;
			collector.collect_components(components_list);
			std::sort(components_list.begin(), components_list.end(), [](ComponentID a, ComponentID b)
				{
					return a > b;
				});

			EntityID eid = add_entity_tagged(components_list, tag);

			set_entity_component<Comp...>(eid, components...);

			return eid;
		}

		void remove_entity(EntityID eid);

		auto get_entities_by_tag(EntityTag tag) { return entity_tags.equal_range(tag); }

		void add_callback(function<void(Manager&, EntityID)> function, EntityID eid);
		void add_callback_current(function<void(Manager&, EntityID)> function)
		{
			ArchetypeX& arch = archetypes.at(current_arch);
			EntityID entity = arch.get_entity_at_id(index);
			add_callback(function, entity);
		}
		void destroy_this()
		{
			add_callback_current([](Manager& manager, EntityID eid)
				{
					manager.remove_entity(eid);
				});
		}
		void destroy_this(EntityID eid)
		{
			add_callback([](Manager& manager, EntityID eid)
				{
					if (manager.has_entity(eid))
						manager.remove_entity(eid);
				}, eid);
		}

		void update();

		void clean()
		{
			vector<EntityID> ids;
			for (auto& id : entities)
				ids.push_back(id.first);

			for (auto& id : ids)
				remove_entity(id);

			entity_counter = 0;
		}
	};

	template<typename T>
	inline T* Manager::get_entity_component(EntityID eid)
	{
		return reinterpret_cast<T*>(get_entity_component_data(eid, TypeId<ComponentX>::id<T>()));
	}

	template<typename T>
	inline void Manager::register_component_type()
	{
		size_t new_id = TypeId<ComponentX>::id<T>();
		factories.at(new_id) = new ComponentXFactory<T>();
		std::cout << "Component " << std::setw(2) << new_id << " added '" << typeid(T).name() << "' " << std::endl;
	}

	template<typename T, typename ...Reqs>
	inline void Manager::register_system(uint8_t layer)
	{
		SystemX* new_system = new SystemXFactory<T, Reqs...>();
		add_system(layer, new_system, TypeId<SystemX>::id<T>());
	}

	template<typename T, typename ...Reqs>
	inline void Manager::register_system_manager(uint8_t layer)
	{
		SystemX* new_system = new SystemXManagerFactory<T, Reqs...>();
		add_system(layer, new_system, TypeId<SystemX>::id<T>());
	}

	template<typename T>
	inline T* Manager::get_system()
	{
		size_t id = TypeId<SystemX>::id<T>();

		assert(systems_by_id.count(id) && "System was not registered");

		SystemX* system = systems_by_id.at(id);

		return reinterpret_cast<T*>(system->get_this());
	}
}