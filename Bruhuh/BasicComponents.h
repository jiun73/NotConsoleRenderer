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

inline ECSX::ComponentXAdder<Position_x> pos_adder;
inline ECSX::ComponentXAdder<Angle_x> angle_adder;
inline ECSX::ComponentXAdder<GFX_x> gfx_adder;
inline ECSX::ComponentXAdder<Controller_x> controller_adder;
inline ECSX::ComponentXAdder<Physics_x> physics_adder;
inline ECSX::ComponentXAdder<Lifetime_x> lifetime_adder;
inline ECSX::ComponentXAdder<ParticleEmitter_x> emitter_adder;
inline ECSX::ComponentXAdder<Distorter_x> distorter_adder;
inline ECSX::ComponentXAdder<Health_x> health_adder;