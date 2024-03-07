#include "pch.h"
#include "BasicSystems.h"
#include "ColliderSystem.h"

namespace PONG
{
	ECSX::SystemXManagerAdder<0, Collision_system, Shape_x, Collider_x, Position_x> collision_system_adder;
	
	//__REGISTER_ENTITYX_SYSTEM_MANAGER__(0, Collision_system);
	__REGISTER_ENTITYX_SYSTEM__(0, Lifetime_system);
	__REGISTER_ENTITYX_SYSTEM__(1, Particle_system);
	__REGISTER_ENTITYX_SYSTEM__(1, Health_system);
	__REGISTER_ENTITYX_SYSTEM__(2, Physics_system);
	__REGISTER_ENTITYX_SYSTEM__(3, Controller_system);
	__REGISTER_ENTITYX_SYSTEM__(4, Shape_system);
	__REGISTER_ENTITYX_SYSTEM__(5, GFX_system);
	
	
	
	

	enum Tags
	{
		TAG_PLAYERS,
	};

	enum cTags
	{
		SOLID =		0b1,	
		PLAYER =	0b10,
		PLAYER2 =	0b100,
		PAD =		0b1000,
	};

	void pong_main()
	{
		ECSX::TaggedEntityX<TAG_PLAYERS, Position_x, Angle_x, GFX_x, Shape_x, Controller_x, Collider_x> player;
		ECSX::TaggedEntityX<TAG_PLAYERS, Position_x, Angle_x, GFX_x, Shape_x, Controller_x, Collider_x> player2;
		ECSX::TaggedEntityX<TAG_PLAYERS, Position_x, Angle_x, GFX_x, Shape_x, Collider_x, Physics_x> ball;
		ECSX::TaggedEntityX<TAG_PLAYERS, Position_x, Angle_x, GFX_x, Shape_x, Collider_x> wall1;
		ECSX::TaggedEntityX<TAG_PLAYERS, Position_x, Angle_x, GFX_x, Shape_x, Collider_x> wall2;

		Rect_d floor = { 0, {700, 10} };
		wall1.create({ {0, 400} }, { 0 }, { rgb(100,100,100) }, { floor }, { PLAYER });
		wall2.create({ {0, -10} }, { 0 }, { rgb(100,100,100) }, { floor }, { PLAYER });

		Rect_d player_square = { {-5,-40}, {10,80} };
		Rect_d ball_square = { 0, {10,10} };
		player.create({ 25 }, { 0 }, { rgb(100,100,100) }, { player_square }, { "p1_" }, { PAD });
		player2.create({ {700, 25} }, { 0 }, { rgb(100,100,100) }, { player_square }, { "p2_" }, { PAD });
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
		entitity_system<Collision_system>()->add_pairing(PAD, SOLID, CTYPE_BOUNCESP);

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

