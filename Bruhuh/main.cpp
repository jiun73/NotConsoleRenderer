#include "TileRenderer.h"
#include "EntityX.h"

double getAngleTowardPoint(V2d_d origin, V2d_d target)
{
	return (-atan2(double(origin.y - target.y), double(origin.x - target.x)) - (M_PI / 2));
}

//struct Object
//{
//	Shape shape;
//	int life = 100;
//	V2d_d vel = 0;
//	bool circle = false;
//	Color color;
//};

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

struct Shape_x 
{
	vector<V2d_i> points;
	V2d_d position = 0;
	double angle = 0;

	V2d_d find_point(V2d_d p)
	{
		double norm = p.norm();
		double orientation = p.orientation() + angle;

		return p.polar(norm, orientation) + position;
	}

	void draw()
	{
		for (auto it = points.begin() + 1; it != points.end(); it++)
		{
			draw_line((V2d_i)find_point(*(it - 1)), (V2d_i)find_point(*it));
		}

		draw_line((V2d_i)find_point(points.front()), (V2d_i)find_point(points.back()));
	}
};

struct Lifetime_x 
{
	int life = 999;
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
		angle->angle = -getAngleTowardPoint(position->position, get_true_mouse_pos()) + (0.5 * M_PI);

		if (mouse().pressed(1))
		{
			EntityX<Position_x, Angle_x, GFX_x, Shape_x, Physics_x, Lifetime_x> bullet;

			bullet.create(
				*position,
				{ random().frange(-2 * M_PI, 2 * M_PI) },
				{ COLOR_WHITE },
				{ { -5, 5, {-5,5 } } },
				{ V2d_d().polar(3, angle->angle), 0, 0.3 },
				{ 100 }
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

SystemXAdder<0, Shape_system, Position_x, Angle_x, Shape_x> shape_system_adder;
SystemXAdder<0, GFX_system, GFX_x, Shape_x> polygon_gfx_system_adder;
SystemXAdder<0, Controller_system, Position_x, Controller_x> controller_system_adder;
SystemXAdder<0, Shooter_system, Shooter_x, Position_x, Angle_x> shooter_system_adder;
SystemXAdder<0, Physics_system, Physics_x, Position_x, Angle_x> physics_system_adder;
SystemXAdder<0, Lifetime_system, Lifetime_x> lifetime_system_adder;
SystemXAdder<1, Particle_system, ParticleEmitter_x, Position_x> particle_system_adder;
SystemXAdder<0, PlayerMoveParticle_system, ParticleEmitter_x, Controller_x> player_particle_system_adder;

int main()
{
	set_window_size(500);
	set_window_resizable();

	init();

	sound().loadSound("Sounds/shoot_sfx1.wav");
	sound().loadSound("Sounds/shoot_sfx2.wav");
	sound().loadSound("Sounds/shoot_sfx3.wav");
	sound().loadSound("Sounds/shoot_sfx4.wav");

	EntityX< Position_x, Angle_x, GFX_x, Controller_x, Shape_x, Shooter_x, ParticleEmitter_x> player;

	player.create(
		{ 250 },
		{ 0 },
		{ rgb(255, 255, 0) },
		{},
		{ { -5,{10,0},{-5,5} } },
		{},
		{ {30} }
	);

	while (run())
	{
		pencil(COLOR_BLACK);
		draw_clear();
	}

	return 0;
}