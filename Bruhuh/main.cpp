#include "TileRenderer.h"
#include "EntityX.h"
#include "Vec3D.h"
#include "Shape_x.h"
#include "Utility.h"
#include <functional>

struct AI_x
{
	int counter = 0;
	int shots = 3;
	int charge = 0;
	int lock = 0;
	bool lock2 = false;
	bool lock3 = false;
	int random = 0;
};

#include "BasicComponents.h"
#include "CollisionSystem.h"

ComponentXAdder<Shape_x> shape_adder;
ComponentXAdder<Collider_x> collider_adder;
ComponentXAdder<AI_x> ai_adder;

#include "BasicSystems.h"

#include "GameGlobalData.h"

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

TaggedEntityX<ENTITY_PLAYER, Position_x, Angle_x, GFX_x, Controller_x, Shape_x, Shooter_x, ParticleEmitter_x, Collider_x, Health_x> player;

struct AI_system 
{
	void update(AI_x* ai, Angle_x* angle, Position_x* position, Physics_x* physic) 
	{
		auto player_range = EntX::get()->get_entities_by_tag(ENTITY_PLAYER);

		if (player_range.first == player_range.second) return;

		V2d_d& player_position = EntX::get()->get_entity_component<Position_x>(player_range.first->second)->position;

		angle->angle = -getAngleTowardPoint(position->position, player_position) + (0.5 * M_PI);

		ai->counter++;

		if (ai->shots > 0 && distance_square(position->position, player_position) < 30000)
		{
			ai->charge = false;

			physic->velocity.x = lerp(physic->velocity.x, 0, 0.05);
			physic->velocity.y = lerp(physic->velocity.y, 0, 0.05);

			if (!ai->lock2)
			{
				ai->random = random().range(14, 30);
				ai->lock2 = false;
			}

			if (ai->counter >= ai->random)
			{
				EntX::get()->add_callback_current([position, angle](EntityID)
					{
						EntityX<Position_x, Angle_x, GFX_x, Shape_x, Physics_x, Lifetime_x, Collider_x> bullet;

						bullet.create(
							*position,
							{ random().frange(-2 * M_PI, 2 * M_PI) },
							{ COLOR_WHITE },
							{ { {-5,5},{10,0}, -5 } },
							{ V2d_d().polar(3, angle->angle), 0, 0 },
							{ 100 },
							{ TAG_EBULLET }
						);

						sound().playSound("Sounds/shoot_sfx" + std::to_string(random().range(1, 4)) + ".wav");
					});

				ai->counter = 0;
				ai->shots--;

				ai->random = random().range(14, 30);
			}

			
		}
		else
		{
			if (!ai->lock2)
			{
				ai->random = random().range(20, 30);
				ai->lock2 = true;
			}

			if (ai->counter >= 100)
			{	
				ai->shots += 3;
				ai->counter = 0;
			}

			if (distance_square(position->position, player_position) > 30000)
			{
				physic->acceleration = V2d_d().polar(0.01, angle->angle);
			}
			
			if (ai->shots > ai->random)
			{
				ai->charge = 1000;
				ai->lock = 4000;
				ai->shots = 4;
			}
			else if (!ai->charge) {

				double top_speed = 1.7;

				if (physic->velocity.x > top_speed)
					physic->velocity.x = top_speed;

				if (physic->velocity.y > top_speed)
					physic->velocity.y = top_speed;

				if (physic->velocity.x < -top_speed)
					physic->velocity.x = -top_speed;

				if (physic->velocity.y < -top_speed)
					physic->velocity.y = -top_speed;
			}

			if (ai->charge)
			{
				physic->velocity.x = lerp(physic->velocity.x, 0, 0.03);
				physic->velocity.y = lerp(physic->velocity.y, 0, 0.03);

				if (ai->lock)
				{
					if (!ai->lock3)
					{
						ai->lock3 = true;
						sound().playSound("Sounds/charge_sfx.wav");
					}
					physic->acceleration = V2d_d().polar(0.16, angle->angle);
					ai->lock--;
				}

				ai->charge--;
			}
			else
				if (ai->lock3)
				{
					ai->lock3 = false;
				}
		}
	}
};

SystemXAdder<0, Shape_system, Position_x, Angle_x, Shape_x> shape_system_adder;
SystemXAdder<0, GFX_system, GFX_x, Shape_x> polygon_gfx_system_adder;
SystemXAdder<0, Controller_system, Position_x, Controller_x> controller_system_adder;
SystemXAdder<1, Shooter_system, Shooter_x, Position_x, Angle_x> shooter_system_adder;
SystemXAdder<0, Physics_system, Physics_x, Position_x, Angle_x> physics_system_adder;
SystemXAdder<0, Lifetime_system, Lifetime_x> lifetime_system_adder;
SystemXAdder<1, Particle_system, ParticleEmitter_x, Position_x> particle_system_adder;
SystemXAdder<0, PlayerMoveParticle_system, ParticleEmitter_x, Controller_x> player_particle_system_adder;
SystemXAdder<1, Distorter_system, Distorter_x, Shape_x> distorter_system_adder;
SystemXAdder<1, AI_system, AI_x, Angle_x, Position_x, Physics_x> ai_system_adder;
SystemXAdder<1, Health_system, Health_x> health_system_adder;
SystemXManagerAdder<0, Collision_system, Shape_x, Collider_x, Position_x> collision_system_adder;

#include "Weapons.h"

int main()
{
	set_window_size(800);
	set_window_resizable();

	init();

	sound().loadSound("Sounds/shoot_sfx1.wav");
	sound().loadSound("Sounds/shoot_sfx2.wav");
	sound().loadSound("Sounds/shoot_sfx3.wav");
	sound().loadSound("Sounds/shoot_sfx4.wav");

	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_TESTOBJ, TAG_PLAYER_ | TAG_TESTOBJ | TAG_ENEMY__, CTYPE_PUSH);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_TESTOBJ, TAG_PBULLET | TAG_EBULLET, CTYPE_DESTROY);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_PBULLET, TAG_ENEMY__, CTYPE_HURT);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_EBULLET, TAG_PLAYER_, CTYPE_HURT);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_ENEMY__, TAG_PBULLET, CTYPE_DESTROY);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_ENEMY__, TAG_ENEMY__, CTYPE_PUSH);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_PLAYER_, TAG_EBULLET, CTYPE_DESTROY);

	Object<> test_collider;

	load_font("Fonts/Perfect DOS VGA 437 Win.ttf");

	player.create(
		{ 250 },
		{ 0 },
		{ rgb(255, 255, 0) },
		{ 2 },
		{ { {-5,5},{10,0}, -5 } },
		{},
		{ {30} },
		{ TAG_PLAYER_ }, {}
	);

	test_collider.create(
		{ {250,140} },
		{ {-50,{-50,50},50,{50,-50}} },
		{ rgb(255,255,255) },
		{ 0 },
		{ TAG_TESTOBJ }
	);

	auto spawn = []() {EntX::get()->add_entity<Position_x, Shape_x, GFX_x, Angle_x, Collider_x, AI_x, Physics_x, Health_x>(
		{ 300 },
		{ { {-10, -10},{-10,10}, {5,0} } },
		{ COLOR_PINK },
		{ 0 },
		{ TAG_ENEMY__ },
		{ 0 },
		{ 0 }, {}, ENTITY_ENEMY
	); };

	WeaponParser parser;
	parser.computer.bulletType.bulletCreationFunction = []()
		{
			std::cout << "pew!" << std::endl;
		};
	parser.computer.storage = 100;
	while (run())
	{
		pencil(COLOR_CYAN);
		draw_clear();

		/*pencil(COLOR_PINK);

		keyboard().openTextInput();
		draw_text(keyboard().getTextInput(), 10, 2);
		std::vector<WeaponError> errors = parser.load(keyboard().getTextInput());

		for (auto& error : errors)
		{
			Rect dest;
			dest.pos = { (int)error.offset * 24, error.line * 24 };
			dest.sz = 35;
			draw_rect(dest);
			switch (error.type)
			{
			case ERROR_NOT_A_REGISTER:
				std::cout << "Register not available!" << std::endl;
				break;
			case ERROR_NOT_IN_INVENTORY:
				std::cout << "Not enough tokens in inventory!" << std::endl;
				break;
			case ERROR_INVALID_ARGUMENT:
				std::cout << "Argument invalid! expected: ";
				
				switch (error.subtype)
				{
				case ERROR_EXPECTING_VALUE:
					std::cout << "constant or register";
					break;
				case ERROR_EXPECTING_REGISTER:
					std::cout << "register";
					break;
				case ERROR_EXPECTING_ADDRESS:
					std::cout << "address or register";
					break;
				case ERROR_EXPECTING_ARGUMENT:
					std::cout << "non-empty argument";
					break;
				default:
					break;
				}

				std::cout << std::endl;

				break;
			case ERROR_TOO_MANY_ARGS:
				std::cout << "Too many arguments!" << std::endl;
				break;
			case ERROR_NOT_ENOUGH_ARGS:
				std::cout << "Not enough arguments!" << std::endl;
				break;
			case ERROR_INVALID_INSTRUCTION:
				std::cout << "Instruction doesn't exist!" << std::endl;
				break;
			
			default:
				break;
			}
		}

		if (mouse_left_held())
		{
			parser.computer.registers.resize(27);
			parser.computer.tick();
		}

		Rect dest;
		dest.pos = { 0, (int)parser.computer.programCounter * 24 };
		dest.sz = 35;
		draw_rect(dest);

		for (size_t i = 0; i < parser.computer.registers.size(); i++)
		{
			char c = 'A' + i;
			string s = " ";
			s.at(0) = c;

			string ss = "" + s + ": " + std::to_string(parser.computer.registers.at(i).value);
			draw_text(ss, { 0,300 + (int)i * 24 }, 2);
		}*/

		draw_text("Lorem ipsum dolor sit amet. Qui minima voluptate est quia tenetur est perspiciatis corporis. Quo consequatur neque eum aperiam laborum a vero quam nam illo minima non iste enim aut accusantium sunt. Et voluptatem voluptatem et aliquam aliquam ut libero enim.Et quasi aspernatur non modi soluta aut velit voluptatem et quia nihil aut sequi nulla ex ducimus aliquid nam enim dolor.Et voluptates quis id magnam ipsa sit impedit voluptates qui asperiores nihil in porro autem aut distinctio repudiandae aut labore itaque.Ut tempore galisum cum aperiam corporis et enim maxime non animi quisquam qui soluta nulla.Est beatae rerum qui culpa omnis vel maxime magnam et maxime tenetur vel corporis autem sit nostrum ipsa 33 velit labore.Qui assumenda nihil et necessitatibus debitis qui tempora sunt hic adipisci aliquid et deleniti nemo ad rerum sequi.Eum asperiores maiores a eius recusandae ad earum illum.Est sequi repudiandae ut placeat aliquid in provident minima ut voluptatem corporis.",0, get_font(0));

		//fonts().get(0).render_atlas();
	}

	return 0;
}