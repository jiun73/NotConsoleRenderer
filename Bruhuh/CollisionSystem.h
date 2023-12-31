#pragma once
#include "CollisionComponent.h"
struct Collision_system 
{
	GJK gjk;

	map< ColliderTag, ColliderKey> masks;
	map< ColliderPair, CollisionTypes> types;

	void add_pairing(ColliderTag tag, ColliderTag collidesWith, CollisionTypes type = CTYPE_CUSTOM)
	{
		masks[tag] |= collidesWith;

		for (size_t i = 0; i < 32; i++)
		{
			size_t m = (1ull << i);
			size_t t = collidesWith & m;
			if (t)
			{
				ColliderPair pair = ((uint64_t)tag << 32) | t;
				types[pair] = type;
			}
		}

		std::cout << "Collider tag " << std::bitset<32>(tag) << " now " << std::bitset<32>(masks[tag]) << std::endl;
	}

	void update(vector<Shape_x*>& shapes, vector<Collider_x*>& colliders, vector<Position_x*>& positions, vector<EntityID>& ids)
	{
		size_t i = 0;
		for (auto it1 = shapes.begin(); it1 != shapes.end(); it1++) //TODO: shitty way to do this
		{
			ColliderTag tag1 = colliders.at(i)->tag;
			ColliderKey key = masks[tag1];

			size_t y = 0;
			for (auto it2 = shapes.begin(); it2 != shapes.end(); it2++)
			{
				ColliderTag tag2 = colliders.at(y)->tag;
				if (it1 != it2 && tag2& key)
				{
					if (gjk.find(**it1, **it2))
					{
						//std::cout << ids.at(i) << " collided with " << ids.at(y) << std::endl;

						ColliderPair pair = ((uint64_t)tag1 << 32) | tag2;
						switch (types[pair])
						{
						case CTYPE_CUSTOM:
							break;
						case CTYPE_PUSH:
							positions.at(y)->position += gjk.EPA(**it1, **it2);
							break;
						case CTYPE_DESTROY:
							EntX::get()->destroy_this(ids.at(y));
							break;
						case CTYPE_HURT:
							if (EntX::get()->entity_has_component<Health_x>(ids.at(y)))
							{
								EntX::get()->get_entity_component < Health_x>(ids.at(y))->health--;

								//if (ids.at(y) == player.get_id())
								//{
								//	global_shape_transform = [](V2d_d point)
								//		{
								//			uint32_t c = SDL_GetTicks() - global_shape_transform_dt;

								//			if (c > 1000) global_shape_transform = nullptr;

								//			double delta = (1000 - std::min(c, uint32_t(1000))) / 1000.0;
								//			return point + V2d_d(random().range(0, 15 * delta), random().range(0, 15 * delta));
								//		};
								//	global_shape_transform_dt = SDL_GetTicks();
								//}

							}
							
							break;
						default:
							break;
						}	
					}
					//gjk.visualize(**it1, **it2);
				}
				y++;
			}
			i++;
		}
	}
};
