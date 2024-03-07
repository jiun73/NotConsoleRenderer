#pragma once
#include "Singleton.h"
#include "ECSX_Manager.h"

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

namespace ECSX {
	typedef Singleton<Manager> Global;
	typedef Global EntX;

	template<size_t Tag, typename... Reqs>
	class TaggedEntityX
	{
	private:
		EntityID id = 0;

		bool created = false;

	public:
		TaggedEntityX() {}
		~TaggedEntityX() {}

		EntityID get_id() { return id; }

		template<typename T>
		T* component() { return EntX::get()->get_entity_component<T>(id); }

		void create(const Reqs&... args)
		{
			id = EntX::get()->add_entity<Reqs...>(args..., Tag);
		}

		void destroy()
		{
			EntX::get()->remove_entity(id);
			created = false;
		}
	};

	template<typename... Reqs>
	using EntityX = TaggedEntityX<0, Reqs...>;

	template<typename T>
	struct ComponentXAdder
	{
		ComponentXAdder() { EntX::get()->register_component_type<T>(); }
		~ComponentXAdder() {}
	};

	template<size_t L, typename T, typename... Reqs>
	struct SystemXAdder
	{
		tuple<ComponentXAdder<Reqs>...> component_adders;

		SystemXAdder() { EntX::get()->register_system<T, Reqs...>(L); }
		~SystemXAdder() {}
	};

	namespace hidden {
		template<size_t L, typename T, typename F>
		struct RegisterSystem_helper;

		template<size_t L, typename T, typename R, typename... Ts>
		struct RegisterSystem_helper<L, T, R(T::*)(Ts*...)> : public SystemXAdder<L, T, Ts...> {};
	}

	template<size_t L, typename T>
	struct RegisterSystem : public hidden::RegisterSystem_helper<L, T, decltype(&T::update)> {};

	template<size_t L, typename T, typename... Reqs>
	struct SystemXManagerAdder
	{
		SystemXManagerAdder() { EntX::get()->register_system_manager<T, Reqs...>(L); }
		~SystemXManagerAdder() {}
	};

	namespace hidden {
		template<size_t L, typename T, typename F>
		struct RegisterSystemManager_helper;

		template<size_t L, typename T, typename R, typename... Ts>
		struct RegisterSystemManager_helper<L, T, R(T::*)(vector<Ts*>&..., vector<EntityID>&)> : public SystemXManagerAdder<L, T, Ts...> {};
	}

	template<size_t L, typename T>
	struct RegisterSystemManager : public hidden::RegisterSystemManager_helper<L, T, decltype(&T::update)> {};

#define CONCAT_IMPL( x, y ) x##y
#define MACRO_CONCAT( x, y ) CONCAT_IMPL( x, y )
#define __REGISTER_ENTITYX_SYSTEM__(layer, type) inline ::ECSX::RegisterSystem<layer, type> MACRO_CONCAT(system_adder, __COUNTER__)
#define __REGISTER_ENTITYX_SYSTEM_MANAGER__(layer, type) inline ::ECSX::RegisterSystem<layer, type> MACRO_CONCAT(system_adder, __COUNTER__)

	inline Manager* manager() 
	{
		return Global::get();
	}
}

