#pragma once

#include "BasicComponents.h"
#include "Shape.h"
#include "ColliderSystem.h"
//#include "Utility.h"

struct Shape_system
{
	void update(Position_x* pos, Angle_x* angle, Shape_x* shape)
	{
		shape->angle = angle->angle;
		shape->position = pos->position;
		ECSX::EntX::get()->add_callback([shape, angle, pos](ECSX::Manager&, ECSX::EntityID)
			{
				shape->angle = angle->angle;
				shape->position = pos->position;
			}, 0);

	}
};

struct GFX_system
{
	V2d_i offset = 0;

	void update(GFX_x* gfx, Shape_x* shape)
	{
		pencil(gfx->col);
		ECSX::EntX::get()->add_callback([shape](ECSX::Manager&, ECSX::EntityID)
			{
				//draw_rect(shape->r);
				shape->draw(camera());
			}, 0);

	}
};

struct Physics_system
{
	void update(Physics_x* physics, Position_x* position, Angle_x* angle)
	{

		//physics->velocity += physics->acceleration + physics->gravity;

		//for (auto& f : physics->forces)
		//{
		//	physics->velocity += f;
		//}

		//physics->forces.clear();

		//physics->velocity.range(-physics->top_speed, physics->top_speed);

		position->position += physics->velocity;

		//angle->angle += physics->angular_velocity;


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
			ECSX::EntX::get()->destroy_this();
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
			draw_circle(to_game(p.pos), p.life / 3);
		}
	}
};

struct Health_system
{
	void update(Health_x* health)
	{
		if (health->health <= 0)
			ECSX::EntX::get()->destroy_this();
	}
};

//struct Shape_system
//{
//	void update(Position_x* pos, Angle_x* ang, Shape_x* sha)
//	{
//		sha->r.pos = pos->position;
//	}
//};