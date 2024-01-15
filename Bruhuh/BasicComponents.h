#pragma once

#include "Vec2D.h"
#include "Color.h"

#include "Shape_x.h"

#include <vector>
#include <functional>

using std::vector;
using std::function;

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

struct Distorter_x
{
	function<void(V2d_d&, size_t)> transform;
	bool set = false;
	Shape_x old_shape;

};

struct Health_x
{
	int health = 10;
};

inline ComponentXAdder<Position_x> pos_adder;
inline ComponentXAdder<Angle_x> angle_adder;
inline ComponentXAdder<GFX_x> gfx_adder;
inline ComponentXAdder<Controller_x> controller_adder;
inline ComponentXAdder<Physics_x> physics_adder;
inline ComponentXAdder<Lifetime_x> lifetime_adder;
inline ComponentXAdder<ParticleEmitter_x> emitter_adder;
inline ComponentXAdder<Distorter_x> distorter_adder;
inline ComponentXAdder<Health_x> health_adder;