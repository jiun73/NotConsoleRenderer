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
};

enum ColliderTags
{
	TAG_PLAYER_ = 0b0001,
	TAG_TESTOBJ = 0b0010,
	TAG_PBULLET = 0b0100,
	TAG_ENEMY__ = 0b1000
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
				{ V2d_d().polar(3, angle->angle), 0, 0 },
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

struct AI_system 
{
	void update() 
	{

	}
};

SystemXAdder<1, Shape_system, Position_x, Angle_x, Shape_x> shape_system_adder;
SystemXAdder<1, GFX_system, GFX_x, Shape_x> polygon_gfx_system_adder;
SystemXAdder<1, Controller_system, Position_x, Controller_x> controller_system_adder;
SystemXAdder<1, Shooter_system, Shooter_x, Position_x, Angle_x> shooter_system_adder;
SystemXAdder<1, Physics_system, Physics_x, Position_x, Angle_x> physics_system_adder;
SystemXAdder<1, Lifetime_system, Lifetime_x> lifetime_system_adder;
SystemXAdder<2, Particle_system, ParticleEmitter_x, Position_x> particle_system_adder;
SystemXAdder<1, PlayerMoveParticle_system, ParticleEmitter_x, Controller_x> player_particle_system_adder;
SystemXAdder<2, Distorter_system, Distorter_x, Shape_x> distorter_system_adder;
SystemXManagerAdder<0, Collision_system, Shape_x, Collider_x, Position_x> collision_system_adder;

template<typename... Ts>
using Object = EntityX<Position_x, Shape_x, GFX_x, Angle_x, Collider_x, Ts...>;

int main()
{
	set_window_size(500);
	set_window_resizable();

	init();

	sound().loadSound("Sounds/shoot_sfx1.wav");
	sound().loadSound("Sounds/shoot_sfx2.wav");
	sound().loadSound("Sounds/shoot_sfx3.wav");
	sound().loadSound("Sounds/shoot_sfx4.wav");

	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_TESTOBJ, TAG_PLAYER_ | TAG_TESTOBJ, CTYPE_PUSH);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_PBULLET, TAG_TESTOBJ, CTYPE_PUSH);

	EntityX< Position_x, Angle_x, GFX_x, Controller_x, Shape_x, Shooter_x, ParticleEmitter_x, Collider_x> player;
	Object<> test_collider;
	Object<> enemy;
	//Object<> test_collider2;

	//std::cout << "first test: " << std::endl;

	//Shape_x shape({ -50,{-50,50},50,{50,-50} });

	//V2d_d buf1 = 0;
	//V2d_d buf2 = 0;
	//for (int i = 0; i < 1000000; i++)
	//{
	//	double angle = 2 * M_PI * (i / 1000000.0);
	//	V2d_d sup1 = shape.support(angle);
	//	V2d_d sup2 = shape.support_op(angle);

	//	if (sup1 != buf1)
	//	{
	//		std::cout << "1 changed " << buf1 << sup1 << " angle: " << angle << std::endl;
	//		buf1 = sup1;
	//	}

	//	if (sup2 != buf2)
	//	{
	//		std::cout << "2 changed " << buf2 << sup2 << " angle: " << angle << std::endl;
	//		buf2 = sup2;
	//	}

	//	//if (sup1 != sup2)
	//		//std::cout << sup1 << ", " << sup2 << " angle: " << angle << std::endl;
	//}

	//for (int i = -1000000; i < 1000000; i++)
	//{
	//	double angle = 2 * M_PI * (i / 500000.0);
	//	V2d_d sup1 = shape.support(angle);
	//	V2d_d sup2 = shape.support_op(angle);
	//	if (sup1 != sup2)
	//		std::cout << sup1 << ", " << sup2 << " angle: " << angle << std::endl;
	//}

	//std::cout << "second test: " << std::endl;

	//Shape_x shape2({ {-5,5},{10,0}, -5 });

	//for (int i = -1000000; i < 1000000; i++)
	//{
	//	double angle = 2 * M_PI * (i / 500000.0);
	//	V2d_d sup1 = shape2.support(angle);
	//	V2d_d sup2 = shape2.support_op(angle);
	//	if (sup1 != sup2)
	//		std::cout << sup1 << ", " << sup2 << " angle: " << angle << std::endl;
	//}

	//std::cout << "third test: " << std::endl;

	//Shape_x shape3({ -5,{10,0}, {-5,5} });

	//for (int i = -1000000; i < 1000000; i++)
	//{
	//	double angle = 2 * M_PI * (i / 500000.0);
	//	V2d_d sup1 = shape2.support(angle);
	//	V2d_d sup2 = shape2.support_op(angle);
	//	if (sup1 != sup2)
	//		std::cout << sup1 << ", " << sup2 << " angle: " << angle << std::endl;
	//}


	player.create(
		{ 250 },
		{ 0 },
		{ rgb(255, 255, 0) },
		{},
		{ { {-5,5},{10,0}, - 5 } },
		{},
		{ {30} },
		{ TAG_PLAYER_ }
	);

	enemy.create(
		{ 30 },
		{ {get_letter_shape('0')} },
		{ COLOR_PINK },
		{0},
		{TAG_ENEMY__}
	);

	while (run())
	{
		pencil(COLOR_BLACK);
		draw_clear();

		pencil(COLOR_PINK);

		//draw_text("How do you do?", 10, 2);

	}

	/*test_collider.create(
		{ 250 }, 
		{ {-50,{-50,50},50,{50,-50}} }, 
		{rgb(255,255,255)},
		{ 0 },
		{ TAG_TESTOBJ }
	);*/

	/*test_collider2.create(
		{ {250,140} },
		{ {-50,{-50,50},50,{50,-50}} },
		{ rgb(255,255,255) },
		{ 0 },
		{ TAG_TESTOBJ }
	);*/

	
	

	return 0;
}