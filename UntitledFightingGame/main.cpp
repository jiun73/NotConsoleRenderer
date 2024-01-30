#include "TileRenderer.h"
#include "EntityX.h"
#include "BasicSystems.h"
#include "ColliderSystem.h"
#include "Shape.h"

#include "Generics.h"
#include "ObjectGenerics.h"
#include "FunctionGenerics.h"
#include "ContainerGenerics.h"
#include "CstarParser.h"

static ComponentXAdder<Shape_x> shape_adder;
static ComponentXAdder<Collider_x> coll_adder;

SystemXAdder<8, Shape_system, Position_x, Angle_x, Shape_x> shape_system_adder;
SystemXAdder<9, GFX_system, GFX_x, Shape_x> polygon_gfx_system_adder;
SystemXAdder<0, Controller_system, Position_x, Controller_x> controller_system_adder;
//SystemXAdder<1, Shooter_system, Shooter_x, Position_x, Angle_x> shooter_system_adder;
SystemXAdder<7, Physics_system, Physics_x, Position_x, Angle_x> physics_system_adder;
SystemXAdder<0, Lifetime_system, Lifetime_x> lifetime_system_adder;
SystemXAdder<1, Particle_system, ParticleEmitter_x, Position_x> particle_system_adder;
//SystemXAdder<0, PlayerMoveParticle_system, ParticleEmitter_x, Controller_x> player_particle_system_adder;
//SystemXAdder<1, Distorter_system, Distorter_x, Shape_x> distorter_system_adder;
//SystemXAdder<1, AI_system, AI_x, Angle_x, Position_x, Physics_x> ai_system_adder;
SystemXAdder<1, Health_system, Health_x> health_system_adder;
SystemXManagerAdder<0, Collision_system, Shape_x, Collider_x, Position_x> collision_system_adder;

enum Tags
{
	TAG_PLAYERS,
};

enum cTags
{
	SOLID = 0b1,
	PLAYER = 0b10
};

int main()
{
	TaggedEntityX<TAG_PLAYERS, Position_x, Angle_x, GFX_x, Shape_x, Controller_x, Collider_x, Physics_x> player;
	TaggedEntityX<TAG_PLAYERS, Position_x, Angle_x, GFX_x, Shape_x, Collider_x> floor;

	Rect_d player_square = { -5,5 };
	player.create({ 25 }, { 0 }, { rgb(100,100,100) }, { player_square }, {}, { SOLID }, { 0,{0,0.03} });

	floor.create({ {0,95}, }, { 0 }, { rgb(100,100,100) }, { {0, {100,0}, {100,-5}, {0,-5}} }, { PLAYER });



	entitity_system<Collision_system>()->add_pairing(PLAYER, SOLID, CTYPE_PUSH);

	set_window_size({ 192 ,108 });
	set_window_resizable();

	track_variable(player.component<Position_x>()->position, "pos");

	while (run())
	{
		pencil(rgb(0, 0, 0));
		draw_clear();
	}
}