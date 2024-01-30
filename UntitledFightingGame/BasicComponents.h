#pragma once

#include "Vec2D.h"
#include "Color.h"

#include "TileRenderer.h"
#include "EntityX.h"

#include <vector>
#include <functional>

using std::vector;
using std::function;

struct Physics_x
{
private:
	vector<V2d_d> forces;

public:
	Physics_x() {}
	Physics_x(const V2d_d& vel, const V2d_d& acc, const V2d_d& grav = 0) : velocity(vel), acceleration(acc), gravity(grav){}
	~Physics_x() {}

	V2d_d velocity;
	V2d_d acceleration;
	V2d_d gravity;

	V2d_d top_speed = 100;

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

struct Particle
{
	int life = 0;
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

struct Health_x
{
	int health = 10;
};

static ComponentXAdder<Position_x> pos_adder;
static ComponentXAdder<Angle_x> angle_adder;
static ComponentXAdder<GFX_x> gfx_adder;
static ComponentXAdder<Controller_x> controller_adder;
static ComponentXAdder<Physics_x> physics_adder;
static ComponentXAdder<Lifetime_x> lifetime_adder;
static ComponentXAdder<ParticleEmitter_x> emitter_adder;
static ComponentXAdder<Health_x> health_adder;