#pragma once

#include "BasicComponents.h"
#include "Shape_x.h"
#include "CollisionSystem.h"
#include "Utility.h"

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
				{ get_letter_shape('G') },
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

struct Health_system
{
	void update(Health_x* health)
	{
		if (health->health <= 0)
			EntX::get()->destroy_this();
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