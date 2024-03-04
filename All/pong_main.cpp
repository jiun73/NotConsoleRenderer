#include "pch.h"
#include "BasicSystems.h"
#include "ColliderSystem.h"

namespace PONG
{
	static ComponentXAdder<Shape_x> shape_adder;
	static ComponentXAdder<Collider_x> coll_adder;
	static ComponentXAdder<Position_x> pos_adder;
	static ComponentXAdder<Angle_x> angle_adder;
	static ComponentXAdder<GFX_x> gfx_adder;
	static ComponentXAdder<Controller_x> controller_adder;
	static ComponentXAdder<Physics_x> physics_adder;
	static ComponentXAdder<Lifetime_x> lifetime_adder;
	static ComponentXAdder<ParticleEmitter_x> emitter_adder;
	static ComponentXAdder<Health_x> health_adder;

	SystemXAdder<9, Shape_system, Position_x, Angle_x, Shape_x> shape_system_adder;
	SystemXAdder<10, GFX_system, GFX_x, Shape_x> polygon_gfx_system_adder;
	SystemXAdder<8, Controller_system, Position_x, Controller_x, Collider_x, Angle_x> controller_system_adder;
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
		PLAYER = 0b10,
		PLAYER2 = 0b100
	};

	void pong_main()
	{
		TaggedEntityX<TAG_PLAYERS, Position_x, Angle_x, GFX_x, Shape_x, Controller_x, Collider_x> player;
		TaggedEntityX<TAG_PLAYERS, Position_x, Angle_x, GFX_x, Shape_x, Controller_x, Collider_x> player2;
		TaggedEntityX<TAG_PLAYERS, Position_x, Angle_x, GFX_x, Shape_x, Collider_x, Physics_x> ball;
		TaggedEntityX<TAG_PLAYERS, Position_x, Angle_x, GFX_x, Shape_x, Collider_x> wall1;
		TaggedEntityX<TAG_PLAYERS, Position_x, Angle_x, GFX_x, Shape_x, Collider_x> wall2;

		Rect_d floor = { 0, {700, 10} };
		wall1.create({ {0, 400} }, { 0 }, { rgb(100,100,100) }, { floor }, { PLAYER });
		wall2.create({ {0, -10} }, { 0 }, { rgb(100,100,100) }, { floor }, { PLAYER });

		Rect_d player_square = { {-5,-40}, {10,80} };
		Rect_d ball_square = { 0, {10,10} };
		player.create({ 25 }, { 0 }, { rgb(100,100,100) }, { player_square }, { "p1_" }, { PLAYER });
		player2.create({ {700, 25} }, { 0 }, { rgb(100,100,100) }, { player_square }, { "p2_" }, { PLAYER });
		ball.create({ {90,95}, }, { 0 }, { rgb(100,100,100) }, { ball_square }, { SOLID }, { {2,2},0 });

		inputs().map("p1_up", { KEYBOARD_INPUT, INPUT_HELD, SDL_SCANCODE_W });
		inputs().map("p1_down", { KEYBOARD_INPUT, INPUT_HELD, SDL_SCANCODE_S });
		inputs().map("p1_rot1", { KEYBOARD_INPUT, INPUT_HELD, SDL_SCANCODE_A });
		inputs().map("p1_rot2", { KEYBOARD_INPUT, INPUT_HELD, SDL_SCANCODE_D });

		inputs().map("p2_up", { KEYBOARD_INPUT, INPUT_HELD, SDL_SCANCODE_UP });
		inputs().map("p2_down", { KEYBOARD_INPUT, INPUT_HELD, SDL_SCANCODE_DOWN });
		inputs().map("p2_rot1", { KEYBOARD_INPUT, INPUT_HELD, SDL_SCANCODE_LEFT });
		inputs().map("p2_rot2", { KEYBOARD_INPUT, INPUT_HELD, SDL_SCANCODE_RIGHT });

		entitity_system<Collision_system>()->add_pairing(PLAYER, SOLID, CTYPE_BOUNCE);
		//entitity_system<Collision_system>()->add_pairing(PLAYER, PLAYER, CTYPE_PUSH);

		while (run())
		{
			pencil(COLOR_BLACK);
			draw_clear();

			if (mouse_left_pressed())
			{
				ball.component<Position_x>()->position = 100;
			}
		}
	}

	GLUU_IMPORT_MAIN(pong_main);
}

