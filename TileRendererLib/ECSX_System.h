#pragma once
#include "ECSX_Archetype.h"
#include <map>

namespace ECSX 
{
	using std::map;

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

		void create() override
		{
			vector<ComponentID> list;

			ComponentCollector<Reqs...> collector;
			collector.collect_components(list);

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

		void update(const ArchetypeX& arch, const map<ComponentID, size_t>& sizes, size_t& index) override
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

		char* get_this() override { return reinterpret_cast<char*>(&system); }
	};

	template<typename T, typename... Reqs>
	class SystemXManagerFactory : public SystemX
	{
		T system;
		tuple<vector<Reqs*>...> collected_data;
		vector<EntityID> ids;

		char* get_this() override { return reinterpret_cast<char*>(&system); }

		void create() override
		{
			vector<ComponentID> list;

			ComponentCollector<Reqs...> collector;
			collector.collect_components(list);

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


}

#include "pch.h"

