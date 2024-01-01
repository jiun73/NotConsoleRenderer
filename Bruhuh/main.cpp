#include "TileRenderer.h"
#include "EntityX.h"
#include "Vec3D.h"
#include "Shape_x.h"
#include "Utility.h"
#include <functional>

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

#include "BasicComponents.h"
#include "CollisionSystem.h"

ComponentXAdder<Shape_x> shape_adder;
ComponentXAdder<Collider_x> collider_adder;
ComponentXAdder<AI_x> ai_adder;

#include "BasicSystems.h"

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
					{ V2d_d().polar(3, angle->angle), 0, 0 },
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
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_EBULLET, TAG_PLAYER_, CTYPE_HURT);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_ENEMY__, TAG_PBULLET, CTYPE_DESTROY);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_ENEMY__, TAG_ENEMY__, CTYPE_PUSH);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_PLAYER_, TAG_EBULLET, CTYPE_DESTROY);

	Object<> test_collider;
	TaggedEntityX<1, Health_x> test;

	test.create({ 789 });

	auto range = EntX::get()->get_entities_by_tag(1);

	test_collider.create(
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

	auto spawn = []() {EntX::get()->add_entity<Position_x, Shape_x, GFX_x, Angle_x, Collider_x, AI_x, Physics_x, Health_x>(
		{ 300 },
		{ { {-10, -10}, {0,-3}, {5,0}, {0,3},{-10,10},0 } },
		{ COLOR_PINK },
		{ 0 },
		{ TAG_ENEMY__ },
		{ 0 },
		{ 0 }, {}, ENTITY_ENEMY
	); };

	spawn();
	spawn();
	spawn();
	spawn();
	spawn();
	spawn();

	for (auto it = range.first; it != range.second; it++)
	{
		Health_x* health = EntX::get()->get_entity_component<Health_x>(it->second);

		std::cout << health->health << std::endl;
	}

	while (run())
	{
		pencil(COLOR_BLACK);
		draw_clear();

		pencil(COLOR_PINK);

		draw_text("LOAD A B\nADD A B A", 10, 2);
	}

	return 0;
}