#pragma once

#include "BasicComponents.h"
#include "Shape.h"
//#include "CollisionSystem.h"
//#include "Utility.h"

struct Shape_system
{
	void update(Position_x* pos, Angle_x* angle, Shape_x* shape)
	{
		std::cout << "shape" << std::endl;
		shape->angle = angle->angle;
		shape->position = pos->position;
		EntX::get()->add_callback([shape, angle, pos](EntityID)
			{
				std::cout << "shape" << shape->position;
				std::cout << "shape" << pos->position << std::endl;
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
		EntX::get()->add_callback([shape](EntityID)
			{
				std::cout << "gfx " << std::endl;
				shape->draw(camera());
			}, 0);
		
	}
};

struct Controller_system
{
	void update(Position_x* position, Controller_x* controller)
	{
		std::cout << "controls" << std::endl;
		V2d_d old = position->position;
		keyboard().quickKeyboardControlWASD(position->position, controller->force);
		controller->displacement_vector = old - position->position;
		std::cout << old << position->position << std::endl;
	}
};

struct Physics_system
{
	void update(Physics_x* physics, Position_x* position, Angle_x* angle)
	{
		std::cout << "physics" << std::endl;
		position->position += physics->velocity;
		physics->velocity += physics->acceleration;
		angle->angle += physics->angular_velocity;
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
			draw_circle(to_game(p.pos), p.life / 3);
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