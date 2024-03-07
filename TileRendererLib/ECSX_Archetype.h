#pragma once
#include "ECSX_ComponentX.h"
#include <unordered_map>

namespace ECSX
{
	using std::unordered_map;

	typedef size_t EntityID;
	typedef size_t EntityTag;
	typedef size_t SystemID;

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

			return 0;
		}
	};
}