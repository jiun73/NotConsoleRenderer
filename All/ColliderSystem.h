#pragma once
#include <type_traits>

enum CollisionTypes
{
	CTYPE_CUSTOM,
	CTYPE_PUSH,
	CTYPE_DESTROY,
	CTYPE_HURT,
	CTYPE_BOUNCE
};

enum ColliderTags
{
	TAG_PLAYER_ = 0b00001,
	TAG_TESTOBJ = 0b00010,
	TAG_PBULLET = 0b00100,
	TAG_ENEMY__ = 0b01000,
	TAG_EBULLET = 0b10000
};

typedef uint32_t ColliderTag;
typedef uint32_t ColliderKey;
typedef uint64_t ColliderPair;

struct Collider_x
{
	ColliderTag tag;
	bool collide_up = false;
	bool collide_down = false;
	bool collide_right = false;
	bool collide_left = false;

	//std::function<void(EntityID, EntityID)> custom_function;
};

struct Controller_system
{
	void update(Position_x* position, Controller_x* controller, Collider_x* collider, Angle_x* ang)
	{
		if (input(controller->input_prefix + "up"))
		{
			position->position.y -= 5;
		}

		if (input(controller->input_prefix + "down"))
		{
			position->position.y += 5;
		}

		if (input(controller->input_prefix + "rot1"))
		{
			ang->angle += M_PI / 25.0;
		}
		if (input(controller->input_prefix + "rot2"))
		{
			ang->angle -= M_PI / 25.0;
		}

		//V2d_d old = position->position;
		////keyboard().quickKeyboardControlWASD(position->position, controller->force);

		//controller->displacement_vector = old - position->position;

		/*if (input(controller->input_prefix + "up"))
		{
			phys->acceleration.x = -100;
		}

		else if (input(controller->input_prefix + "down"))
		{
			phys->acceleration.x = 100;
		}
		else
		{
			phys->acceleration.x = 0;
			phys->velocity.x = 0;
		}

		if (input(controller->input_prefix + "jump") && collider->collide_down)
		{
			phys->forces.push_back({ 0,-1.3 });
		}*/
	}
};

#include "BasicComponents.h"
#include "Shape.h"

struct Collision_system
{
	GJK gjk;

	map< ColliderTag, ColliderKey> masks;
	map< ColliderPair, CollisionTypes> types;
	map< ColliderPair, std::function<void(EntityID, EntityID)>> customs;

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

		for (auto it1 = shapes.begin(); it1 != shapes.end(); it1++)
		{
			colliders.at(i)->collide_down = false;
			colliders.at(i)->collide_up = false;
			colliders.at(i)->collide_left = false;
			colliders.at(i)->collide_right = false;
			i++;
		}

		i = 0;
		for (auto it1 = shapes.begin(); it1 != shapes.end(); it1++) //TODO: shitty way to do this
		{
			ColliderTag tag1 = colliders.at(i)->tag;
			ColliderKey key = masks[tag1];

			size_t y = 0;
			for (auto it2 = shapes.begin(); it2 != shapes.end(); it2++)
			{
				ColliderTag tag2 = colliders.at(y)->tag;
				if (it1 != it2 && tag2 & key)
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
						{
							V2d_d dis = gjk.EPA(**it1, **it2);
							size_t id = ids.at(y);
							positions.at(y)->position += dis;

							if (EntX::get()->entity_has_component<Physics_x>(id))
							{
								Physics_x* physics = EntX::get()->get_entity_component < Physics_x>(id);

								if (dis.y < 0 && physics->velocity.y > 0)
								{
									colliders.at(y)->collide_down = true; physics->velocity.y = 0;
								}

								if (dis.y > 0 && physics->velocity.y < 0)
								{
									colliders.at(y)->collide_up = true; physics->velocity.y = 0;
								}
								if (dis.x < 0 && physics->velocity.x > 0) { colliders.at(y)->collide_right = true; physics->velocity.x = 0; }
								if (dis.x > 0 && physics->velocity.x < 0) { colliders.at(y)->collide_left = true; physics->velocity.x = 0; }
							}
						}
						break;
						case CTYPE_DESTROY:
							EntX::get()->destroy_this(ids.at(y));
							break;
						case CTYPE_HURT:
							if (EntX::get()->entity_has_component<Health_x>(ids.at(y)))
							{
								EntX::get()->get_entity_component < Health_x>(ids.at(y))->health--;
							}

							break;

						case CTYPE_BOUNCE:
						{
							V2d_d dis = gjk.EPA(**it1, **it2);
							size_t id = ids.at(y);
							positions.at(y)->position += dis;

							if (EntX::get()->entity_has_component<Physics_x>(id))
							{
								Physics_x* physics = EntX::get()->get_entity_component < Physics_x>(id);

								double velang = (-physics->velocity).orientation();
								double normang = dis.orientation();

								double angdiff = normang - velang;
								physics->velocity.polar(1.0, velang + angdiff * 2);
							}
							else
							{
								std::cout << "kys" << std::endl;
							}
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
