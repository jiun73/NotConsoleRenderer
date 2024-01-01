#include "TileRenderer.h"
#include "EntityX.h"
#include "Vec3D.h"
#include "Shape_x.h"
#include <functional>

void apply_shape(Shape_x& shape, function<void(V2d_d&, size_t)> transform)
{
	size_t i = 0;
	for (auto& p : shape.get_points())
	{
		transform(p, i);
		i++;
	}
}

double distance_square(const V2d_d& pos1, const V2d_d& pos2)
{
	return (pos1.x - pos2.x) * (pos1.x - pos2.x) + (pos1.y - pos2.y) * (pos1.y - pos2.y);
}

double distance(const V2d_d& pos1, const V2d_d& pos2)
{
	return sqrt(distance_square(pos1, pos2));
}

Shape_x get_letter_shape(char c)
{
	switch (c)
	{
	case ' ':
		return { {} };
	case '0':
		return { {{{-5,0},{0,-5},{5,0}, {0,5}}} };
	case '1':
		return { {{0,-5},{0,5}} };
	case '2':
		return { {{-5,0},{0,-5},{5,0},{-5, 5}, 5, {-5, 5}, {5,0}, {0,-5}} };
	case '3':
		return { {{-5}, {5, -2.5}, {-5,0}, {5,2.5}, {-5,5}, {5,2.5}, {-5,0}, {5, -2.5} } };
	case '4':
		return { {5, {5, 0},{5,-5},{-5,0},{5,0}} };
	case '5':
		return { { {-5,5},{5,2.5}, {-5,0},-5, {5, -5}, -5, {-5,0},{5,2.5}} };
	case '6':
		return { { -5, {-5, 5}, 5, {-5, 0}} };
	case '7':
		return { {-5, {5, -5}, {-5,5},{5, -5}} };
	case '8':
		return { {{0,-5}, {-5,-2.5}, {5,2.5}, {0,5}, {-5, 2.5}, {5, -2.5}} };
	case '9':
		return { {5,{5,-5},-5, {5,0}} };
	case 'A':
		return { { {-2.5,0}, {-5,5},{0,-5},{5},{2.5,0}} };
	case 'a':
		return { {{0,-2.5},{-5, 1.25 }, {0,5}, {5,1.25},5} };
	case 'B':
		return { {{-5,5},{5,2.5},{-5,0},{5,-2.5},-5} };
	case 'b':
		return { {-5, {-5,5},{5,2.5},{-5,0}} };
	case 'C':
		return { {5,{-5,0},{5,-5},{-5,0}} };
	case 'c':
		return { {5, {-5,1.25}, {5,-2.5},{-5,1.25}} };
	case 'D':
		return { {-5,{-5,5},{5,0}} };
	case 'd':
		return { {{5,-5},5,{-5,2.5},{5,0}} };
	case 'E':
		return { {5,{-5,0},{5,-5},{-5,0},{5,0},{-5,0}} };
	case 'e':
		return { {5, {-5,1.25},{0,-2.5},{5,1.25}, {-5,1.25},5} };
	case 'F':
		return { {{ 5,-5},-5,{-5,0},{5,0},{-5,0},{-5,5},-5} };
	case 'f':
		return { {{5,-2.5},{-5,1.25},{5,1.25},{-5,1.25},{-5,5},{-5,1.25} } };
	case 'G':
		return { {{2.5,-2.5},{0,-5},{-5,0},{0,5},{5,0},0,{5,0}, {0,5}, {-5,0}, {0,-5}} };
	case 'g':
		return { {{{0,-2.5}, {5,-2.5},{0,-2.5},{5,1.25},{-5,6.25},{0,7.5},{5,6.25},{-5, 1.25}}} };
	case 'H':
		return { {{-5,{-5,5},{-5,0},{5,0},5,{5,-5},{5,0},{-5,0}}} };
	case 'h':
		return { {{-5,5},{-5,0},5,{-5,0},-5} };
	case 'I':
		return { {{-5,{5,-5},{0,-5},{0,5},5,{-5,5},{0,5},{0,-5}}} };
	case 'i':
		return { {{{-5, -2.5},{5,-2.5},{0,-2.5},{0,5},{5,5},{-5,5},{0,5},{0,-2.5}}} };
	case 'J':
		return { {{-5,0},{0,5},{5,0},{5,-5},{0,-5},{5,-5},{5,0},{0,5}} };
	case 'j':
		return { {{-2.5, -2.5}, {2.5,-2.5},{2.5,5},{0,7.5},{-2.5,5},{0,7.5},{2.5,5}, {2.5,-2.5}} };
	case 'K':
		return { {{-5,{-5,5},{-5,0},{5,-5},{-5,0},5, {-5,0}}} };
	case 'k':
		return { {{-5,{-5,5},{-5,1.25},{5,1.25},{-5,1.25},5, {-5,1.25}}} };
	case 'L':
		return { {-5, {-5,5},5,{-5,5}} };
	case 'l':
		return { {-5, {-5,5},{2.5,1.25},{-5,5}} };
	case 'M':
		return { {{-5,5},-5,{0,5},{5,-5},5,{5,-5},{0,5},-5} };
	case 'm':
		return { {{-5,5},{-5,-2.5},{0,1.25},{5,-2.5},5,{5,-2.5},{0,1.25},{-5,-2.5}} };
	case 'N':
		return { {{-5,5}, -5, 5,{5,-5}, 5, -5} };
	case 'n':
		return { {{-5, -2.5}, {-5,5},{5, -2.5},5,{5, -2.5}, {-5,5} } };
	case 'O':
		return { {{{-5,0},{0,-5},{5,0}, {0,5}}} };
	case 'o':
		return { {{{-5,1.25},{0,-2.5},{5,1.25}, {0,5}}} };
	case 'P':
		return { {-5, {5,-2.5}, {-5,0},{-5,5}} };
	case 'p':
		return { {{-5,-2.5}, {5,1.25}, {-5,2.5},{-5,7.5}} };
	case 'Q':
		return { {{-5,0},{0,5},2.5, 0, 5, 2.5, {5,0}, {0,-5}} };
	case 'q':
		return { {{5,1.25}, {0,-2.5},{-5,1.25},{0,5},{5,1.25},{5,-2.5},{5,7.5}} };
	case 'R':
		return { {{-5,5},{-5,0},5,{-5,0},{5,-2.5},-5} };
	case 'r':
		return { {{-5,5},{-5,-2.5},{ -5,0},{0,-2.5},{5,0},{0,-2.5},{-5,0} } };
	case 'S':
		return { {{-5,5},{5,2.5},{-5,-2.5},{5,-5},{-5,-2.5},{5,2.5}} };
	case 's':
		return { {{-5,5},{5,1.25},{-5,0},{5,-2.5},{-5,0},{5,1.25}} };
	case 'T':
		return { {-5,{5,-5},{0,-5},{0,5},{0,-5}} };
	case 't':
		return { {{-5, -2.5},{5,-2.5},{0,-2.5},{0,5},{0,-2.5}} };
	case 'U':
		return { {-5, {-5,5},5,{5,-5},5,{-5,5}} };
	case 'u':
		return { {{-5,-2.5}, {-5,5},5,{5,-2.5},5,{-5,5}} };
	case 'V':
		return { { -5, {0,5},{5,-5},{0,5}} };
	case 'v':
		return { { {-5,-2.5}, {0,5},{5,-2.5},{0,5}} };
	case 'W':
		return { {{5,-5},5,{0,-5},{-5,5},-5,{-5,5},{0,-5},5} };
	case 'w':
		return { {{5,-2.5},5,{0,1.25},{-5,5},{-5,-2.5},{-5,5},{0,1.25},5} };
	case 'X':
		return { {-5, 5, 0, {-5,5}, 0 , {5,-5}, 0} };
	case 'x':
		return { {{-5,-2.5}, {0,1.25}, 5, {0,1.25}, {-5,5}, {0,1.25} , {5,-2.5}, {0,1.25}} };
	case 'Y':
		return { {-5, 0, {5,-5}, {-5,5},0} };
	case 'y':
		return { {{-5,-2.5}, {0,1.25}, {5,-2.5}, {-5,5},{0,1.25}} };
	case 'Z':
		return { {{-5, {5,-5}, {-5,5},5, {-5,5}, {5,-5}}} };
	case 'z':
		return { {{{-5,-2.5}, {5,-2.5}, {-5,5},5, {-5,5}, {5,-2.5}}} };
	default:
		return {};
	}
}
void distort_shape(Shape_x& shape, double force = 5)
{
	apply_shape(shape, [&](V2d_d& point, size_t)
		{
			point += {random().frange(-force, force), random().frange(-force, force)};;
		});
}

void wave_shape(Shape_x& shape, double amplitude = 5)
{
	apply_shape(shape, [&](V2d_d& point, size_t i)
		{
			point += {sin((SDL_GetTicks() / 100.0) + i * 5)* amplitude, cos((SDL_GetTicks() / 100.0) + i * 5)* amplitude};;
		});
}

void draw_letter(char c, V2d_i pos, double scale = 1)
{
	Shape_x shape = get_letter_shape(c);

	shape.position = pos;
	shape.scale = scale;

	shape.draw();
}

void draw_text(string s, V2d_i pos, double scale = 1)
{
	int basex = pos.x;
	for (auto& c : s)
	{
		if (c == '\n')
		{
			pos.x = basex;
			pos.y += 12 * scale;
			continue;
		}

		draw_letter(c, pos + 5 * scale, scale);
		pos.x += 12 * scale;
	}
}

enum CollisionTypes
{
	CTYPE_CUSTOM,
	CTYPE_PUSH,
	CTYPE_DESTROY,
	CTYPE_HURT
};

enum ColliderTags
{
	TAG_PLAYER_ = 0b00001,
	TAG_TESTOBJ = 0b00010,
	TAG_PBULLET = 0b00100,
	TAG_ENEMY__ = 0b01000,
	TAG_EBULLET = 0b10000
};

struct Physics_x 
{
	V2d_d velocity;
	V2d_d acceleration;

	double angular_velocity = 0;
};

struct Angle_x 
{
	double angle;
};

struct Position_x 
{
	V2d_d position;
};

struct GFX_x
{
	Color col;
};

struct Controller_x 
{
	int force = 1;
	V2d_d displacement_vector;
};

struct Shooter_x {};

struct Particle 
{
	int life;
	V2d_d pos;
	V2d_d vel;
};

struct ParticleEmitter_x
{
	Particle particle_form;
	vector<Particle> particles = {};
	bool emit = true;
};

struct Lifetime_x 
{
	int life = 999;
};

struct Distorter_x 
{
	function<void(V2d_d&, size_t)> transform;
	bool set = false;
	Shape_x old_shape;
	
};

typedef uint32_t ColliderTag;
typedef uint32_t ColliderKey;
typedef uint64_t ColliderPair;

struct Collider_x 
{
	ColliderTag tag;
};

struct AI_x
{
	int counter = 0;
	int shots = 3;
	int charge = 0;
	int lock = 0;
	bool lock2 = false;
	bool lock3 = false;
	int random = 0;
};

struct Health_x 
{
	int health = 10;
};

ComponentXAdder<Position_x> pos_adder;
ComponentXAdder<Angle_x> angle_adder;
ComponentXAdder<GFX_x> gfx_adder;
ComponentXAdder<Controller_x> controller_adder;
ComponentXAdder<Shape_x> shape_adder;
ComponentXAdder<Shooter_x> shooter_adder;
ComponentXAdder<Physics_x> physics_adder;
ComponentXAdder<Lifetime_x> lifetime_adder;
ComponentXAdder<ParticleEmitter_x> emitter_adder;
ComponentXAdder<Distorter_x> distorter_adder;
ComponentXAdder<Collider_x> collider_adder;
ComponentXAdder<AI_x> ai_adder;
ComponentXAdder<Health_x> health_adder;

EntityX< Position_x, Angle_x, GFX_x, Controller_x, Shape_x, Shooter_x, ParticleEmitter_x, Collider_x, Health_x> player;

struct Shape_system 
{
	void update(Position_x* pos, Angle_x* angle, Shape_x* shape) 
	{
		shape->angle = angle->angle;
		shape->position = pos->position;
	}
};

struct GFX_system 
{
	void update(GFX_x* gfx, Shape_x* shape)
	{
		pencil(gfx->col);
		shape->draw();
	}
};

struct Controller_system 
{
	void update(Position_x* position, Controller_x* controller)
	{
		V2d_d old = position->position;
		keyboard().quickKeyboardControlWASD(position->position, controller->force);
		controller->displacement_vector = old - position->position;
	}
};

struct Physics_system 
{
	void update(Physics_x* physics, Position_x* position, Angle_x* angle)
	{
		position->position += physics->velocity;
		physics->velocity += physics->acceleration;
		angle->angle += physics->angular_velocity;
	}
};

struct Shooter_system 
{
	void update(Shooter_x* shooter, Position_x* position, Angle_x* angle)
	{
		angle->angle = -getAngleTowardPoint(position->position, mouse_position()) + (0.5 * M_PI);

		if (mouse().pressed(1))
		{
			EntityX<Position_x, Angle_x, GFX_x, Shape_x, Physics_x, Lifetime_x, Collider_x> bullet;

			bullet.create(
				*position,
				{ random().frange(-2 * M_PI, 2 * M_PI) },
				{ COLOR_WHITE },
				{ get_letter_shape('G')},
				{ V2d_d().polar(5, angle->angle), 0, 0 },
				{ 100 },
				{ TAG_PBULLET }
			);

			sound().playSound("Sounds/shoot_sfx" + std::to_string(random().range(1, 4)) + ".wav");
		}
	}
};

struct Lifetime_system 
{
	void update(Lifetime_x* lifetime) 
	{
		if (lifetime->life > 0)
		{
			lifetime->life--;
		}
		else if (lifetime->life == 0)
		{
			EntX::get()->destroy_this();
		}
	}
};

struct Particle_system 
{
	void update(ParticleEmitter_x* emitter, Position_x* position) 
	{
		if (emitter->emit && random().range(0, 3) == 3)
		{
			emitter->particle_form.pos = position->position;
			emitter->particles.push_back(emitter->particle_form);
		}

		for (auto it = emitter->particles.begin(); it != emitter->particles.end(); it++)
		{
			if (it->life > 0)
				it->life--;

			it->pos += it->vel;
			it->vel /= 2;

			if (it->life == 0)
				it = emitter->particles.erase(it);

			if (it == emitter->particles.end())
				break;
		}

		for (auto& p : emitter->particles)
		{
			pencil(rainbow(1000));
			draw_circle(p.pos, p.life / 3);
		}
	}
};

struct PlayerMoveParticle_system 
{
	void update(ParticleEmitter_x* emitter, Controller_x* controller)
	{
		if (controller->displacement_vector != 0)
		{
			double ori = controller->displacement_vector.orientation() + random().frange(-0.3, 0.3);

			emitter->emit = true;
			emitter->particle_form.vel.polar(10, ori);
		}
		else
			emitter->emit = false;
	}
};

struct Distorter_system 
{
	void update(Distorter_x* distorter, Shape_x* shape)
	{
		if (!distorter->set)
		{
			distorter->old_shape = *shape;
			distorter->set = true;
		}
		else 
		{
			*shape = distorter->old_shape;
		}

		apply_shape(*shape, distorter->transform);
	}
};

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
						std::cout << ids.at(i) << " collided with " << ids.at(y) << std::endl;

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

								if (ids.at(y) == player.get_id())
								{
									global_shape_transform = [](V2d_d point)
										{
											uint32_t c = SDL_GetTicks() - global_shape_transform_dt;

											if (c > 1000) global_shape_transform = nullptr;

											double delta = (1000 - std::min(c, uint32_t(1000))) / 1000.0;
											return point + V2d_d(random().range(0, 15 * delta), random().range(0, 15 * delta));
										};
									global_shape_transform_dt = SDL_GetTicks();
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

double lerp(double a, double b, double t)
{
	return a + t * (b - a);
}

struct AI_system 
{
	void update(AI_x* ai, Angle_x* angle, Position_x* position, Physics_x* physic) 
	{
		angle->angle = -getAngleTowardPoint(position->position, player.component<Position_x>()->position) + (0.5 * M_PI);

		ai->counter++;

		if (ai->shots > 0 && distance_square(position->position, player.component<Position_x>()->position) < 30000)
		{
			ai->charge = false;

			physic->velocity.x = lerp(physic->velocity.x, 0, 0.05);
			physic->velocity.y = lerp(physic->velocity.y, 0, 0.05);

			if (!ai->lock2)
			{
				ai->random = random().range(14, 30);
				ai->lock2 = false;
			}

			if (ai->counter >= ai->random)
			{
				EntityX<Position_x, Angle_x, GFX_x, Shape_x, Physics_x, Lifetime_x, Collider_x> bullet;

				bullet.create(
					*position,
					{ random().frange(-2 * M_PI, 2 * M_PI) },
					{ COLOR_WHITE },
					{ get_letter_shape('A') },
					{ V2d_d().polar(2, angle->angle), 0, 0 },
					{ 100 },
					{ TAG_EBULLET }
				);

				sound().playSound("Sounds/shoot_sfx" + std::to_string(random().range(1, 4)) + ".wav");

				ai->counter = 0;
				ai->shots--;

				ai->random = random().range(14, 30);
			}

			
		}
		else
		{
			if (!ai->lock2)
			{
				ai->random = random().range(20, 30);
				ai->lock2 = true;
			}

			if (ai->counter >= 100)
			{	
				ai->shots += 3;
				ai->counter = 0;
			}

			if (distance_square(position->position, player.component<Position_x>()->position) > 30000)
			{
				physic->acceleration = V2d_d().polar(0.01, angle->angle);
			}
			
			if (ai->shots > ai->random)
			{
				ai->charge = 1000;
				ai->lock = 4000;
				ai->shots = 4;
			}
			else if (!ai->charge) {

				double top_speed = 1.7;

				if (physic->velocity.x > top_speed)
					physic->velocity.x = top_speed;

				if (physic->velocity.y > top_speed)
					physic->velocity.y = top_speed;

				if (physic->velocity.x < -top_speed)
					physic->velocity.x = -top_speed;

				if (physic->velocity.y < -top_speed)
					physic->velocity.y = -top_speed;
			}

			if (ai->charge)
			{
				physic->velocity.x = lerp(physic->velocity.x, 0, 0.03);
				physic->velocity.y = lerp(physic->velocity.y, 0, 0.03);

				if (ai->lock)
				{
					if (!ai->lock3)
					{
						ai->lock3 = true;
						sound().playSound("Sounds/charge_sfx.wav");
					}
					physic->acceleration = V2d_d().polar(0.16, angle->angle);
					ai->lock--;
				}

				ai->charge--;
			}
			else
				if (ai->lock3)
				{
					ai->lock3 = false;
				}
		}
	}
};

struct Health_system 
{
	void update(Health_x* health)
	{
		if (health->health <= 0)
			EntX::get()->destroy_this();
	}
};

SystemXAdder<0, Shape_system, Position_x, Angle_x, Shape_x> shape_system_adder;
SystemXAdder<0, GFX_system, GFX_x, Shape_x> polygon_gfx_system_adder;
SystemXAdder<0, Controller_system, Position_x, Controller_x> controller_system_adder;
SystemXAdder<0, Shooter_system, Shooter_x, Position_x, Angle_x> shooter_system_adder;
SystemXAdder<0, Physics_system, Physics_x, Position_x, Angle_x> physics_system_adder;
SystemXAdder<0, Lifetime_system, Lifetime_x> lifetime_system_adder;
SystemXAdder<1, Particle_system, ParticleEmitter_x, Position_x> particle_system_adder;
SystemXAdder<0, PlayerMoveParticle_system, ParticleEmitter_x, Controller_x> player_particle_system_adder;
SystemXAdder<1, Distorter_system, Distorter_x, Shape_x> distorter_system_adder;
SystemXAdder<1, AI_system, AI_x, Angle_x, Position_x, Physics_x> ai_system_adder;
SystemXAdder<1, Health_system, Health_x> health_system_adder;
SystemXManagerAdder<0, Collision_system, Shape_x, Collider_x, Position_x> collision_system_adder;

template<typename... Ts>
using Object = EntityX<Position_x, Shape_x, GFX_x, Angle_x, Collider_x, Ts...>;

int main()
{
	set_window_size(800);
	set_window_resizable();

	init();

	sound().loadSound("Sounds/shoot_sfx1.wav");
	sound().loadSound("Sounds/shoot_sfx2.wav");
	sound().loadSound("Sounds/shoot_sfx3.wav");
	sound().loadSound("Sounds/shoot_sfx4.wav");

	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_TESTOBJ, TAG_PLAYER_ | TAG_TESTOBJ | TAG_ENEMY__, CTYPE_PUSH);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_TESTOBJ, TAG_PBULLET, CTYPE_DESTROY);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_PBULLET, TAG_ENEMY__, CTYPE_HURT);
	//EntX::get()->get_system<Collision_system>()->add_pairing(TAG_EBULLET, TAG_PLAYER_, CTYPE_HURT);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_ENEMY__, TAG_PBULLET, CTYPE_DESTROY);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_ENEMY__, TAG_ENEMY__, CTYPE_PUSH);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_PLAYER_, TAG_EBULLET, CTYPE_DESTROY);

	Object<> test_collider;
	Object<AI_x, Physics_x, Health_x> enemy;
	Object<AI_x, Physics_x, Health_x> enemy2;
	Object<AI_x, Physics_x, Health_x> enemy3;
	Object<AI_x, Physics_x, Health_x> enemy4;
	Object<> test_collider2;

	test_collider2.create(
		{ {250,140} },
		{ {-50,{-50,50},50,{50,-50}} },
		{ rgb(255,255,255) },
		{ 0 },
		{ TAG_TESTOBJ }
	);

	player.create(
		{ 250 },
		{ 0 },
		{ rgb(255, 255, 0) },
		{ 2},
		{ { {-5,5},{10,0}, -5 } },
		{},
		{ {30} },
		{ TAG_PLAYER_ }, {}
	);

	enemy.create(
		{ 300 },
		{ { {-10, -10}, {0,-3}, {5,0}, {0,3},{-10,10},0 } },
		{ COLOR_PINK },
		{ 0 },
		{ TAG_ENEMY__ },
		{0},
		{ 0 }, {}
	);

	enemy2.create(
		{ 310 },
		{ { {-10, -10}, {0,-3}, {5,0}, {0,3},{-10,10},0 } },
		{ COLOR_PINK },
		{ 0 },
		{ TAG_ENEMY__ },
		{ 0 },
		{ 0 }, {}
	);

	enemy3.create(
		{ 320 },
		{ { {-10, -10}, {0,-3}, {5,0}, {0,3},{-10,10},0 } },
		{ COLOR_PINK },
		{ 0 },
		{ TAG_ENEMY__ },
		{ 0 },
		{ 0 }, {}
	);

	enemy4.create(
		{ 330 },
		{ { {-10, -10}, {0,-3}, {5,0}, {0,3},{-10,10},0 } },
		{ COLOR_PINK },
		{ 0 },
		{ TAG_ENEMY__ },
		{ 0 },
		{ 0 }, {}
	);

	while (run())
	{
		pencil(COLOR_BLACK);
		draw_clear();

		pencil(COLOR_PINK);

		draw_text("LOAD A B\nADD A B A", 10, 2);

	}

	return 0;
}