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

inline int margin = 200;

enum Fonts 
{
	FONT_PIXEL
};

int main()
{
	set_window_size({800 + margin * 2,800});
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
	parser.computer.storage = 25;
	parser.computer.clockSpeed = 10;

	bool edit = true;
	bool inputWasEmpty = false;
	int current_selected_line = 0;

	const Font& pixel_font = get_font(FONT_PIXEL);

	auto change_line = [&](int n)
		{
			current_selected_line = n;

			current_selected_line = std::max(current_selected_line, 0);
			current_selected_line = std::min(current_selected_line, parser.computer.storage - 1);

			keyboard().getTextInput() = parser.source.at(current_selected_line);
		};

	keyboard().lockInputsInTextmode = false;

	while (run())
	{
		pencil(COLOR_CYAN);
		draw_clear();

		pencil(COLOR_BLACK);
		draw_full_rect({ 0, {margin,800} });
		draw_full_rect({ {800 + margin, 0}, {margin,800} });

		Rect dest;
		dest.pos = { 0, pixel_font.height * ((int)(parser.computer.programCounter)) };
		dest.sz = { margin, pixel_font.height };
		pencil(rgb(100,100,100));
		draw_full_rect(dest);

		size_t i = 0;
		for (auto& lines : parser.source)
		{
			if(parser.computer.instructions.at(i).data)
				set_font_pencil(COLOR_GREEN, pixel_font);
			else
				set_font_pencil(COLOR_WHITE, pixel_font);
			draw_text(lines, 800, {0, (int)i * pixel_font.height}, pixel_font);
			i++;
		}
		
		parser.source.resize(parser.computer.storage);

		if (edit)
		{
			string& input = keyboard().getTextInput();
			keyboard().openTextInput();
			parser.source.at(current_selected_line) = input;

			if (key_pressed(SDL_SCANCODE_RETURN))
			{
				input.pop_back();
				parser.source.at(current_selected_line) = input;

				change_line(current_selected_line + 1);
				if (parser.source.back() == "")
				{
					
					parser.source.insert(parser.source.begin() + current_selected_line, "");
					size_t i = current_selected_line + 1;
					for (auto it = parser.source.begin() + current_selected_line + 1; it != parser.source.end(); it++)
					{
						if (i >= parser.computer.storage) break;
						if (parser.computer.instructions.at(i - 1).data)
						{
							std::swap(parser.source.at(i), parser.source.at(i - 1));
						}
						i++;
					}
					keyboard().getTextInput() = parser.source.at(current_selected_line);
					parser.source.pop_back();
				}
			}

			if (key_pressed(SDL_SCANCODE_BACKSPACE) && input.empty() && inputWasEmpty)
			{
				change_line(current_selected_line - 1);
			}

			if (key_pressed(SDL_SCANCODE_UP))
			{
				change_line(current_selected_line - 1);
			}

			if (key_pressed(SDL_SCANCODE_DOWN))
			{
				change_line(current_selected_line + 1);
			}

			Rect dest;
			dest.pos = { get_text_draw_size(input, pixel_font), current_selected_line * pixel_font.height};
			dest.sz = { pixel_font.get('A').sdl_dest.w, pixel_font.get('A').sdl_dest.h };

			pencil(COLOR_WHITE);
			draw_full_rect(dest);

			inputWasEmpty = input.empty();
		}

		std::vector<WeaponError> errors = parser.load(keyboard().getTextInput());

		
		
		for (size_t i = 0; i < parser.source.size(); i++)
		{
			Rect dest;
			dest.pos = { 0, pixel_font.height * (int)i };
			dest.sz = { margin, pixel_font.height };

			if (point_in_rectangle(mouse_position(), dest))
			{
				if (mouse_left_pressed())
				{
					change_line(i);
				}
				pencil(COLOR_WHITE);
			}
			else
				pencil(COLOR_BLACK);

			draw_rect(dest);
		}

		pencil(COLOR_PINK);

		for (auto& error : errors)
		{
			//string& line_text = lines.at(error.line);
			string line_text = keyboard().getTextInput();
			int text_size;
			char character;
			error.offset -= 2;

			if (line_text.size() > error.offset)
			{
				text_size = hidden::get_text_draw_size(line_text.cbegin(), line_text.cbegin() + error.offset, pixel_font);
				character = line_text.at(error.offset);
			}
			else
			{
				text_size = 0;
				character = 'a';
			}

			Rect dest;
			dest.pos = { text_size, error.line * pixel_font.height };
			dest.sz = {pixel_font.get(character).advance,pixel_font.height};
			draw_rect(dest);

			draw_simple_text(get_error_message(error), { margin, error.line * pixel_font.height }, pixel_font);
		}

		if (mouse_left_held())
		{
			parser.computer.registers.resize(27);
			parser.computer.tick();
		}

		if (mouse_right_pressed())
		{
			parser.computer.registers.resize(27);
			parser.computer.clockLast = SDL_GetTicks() - parser.computer.clockSpeed * 2;
			parser.computer.tick();
		}

		for (size_t i = 0; i < parser.computer.registers.size(); i++)
		{
			char c = 'A' + i;
			string s = " ";
			s.at(0) = c;

			string ss = "" + s + ": " + std::to_string(parser.computer.registers.at(i).value);
			draw_text(ss, 800, { 0,300 + (int)i * 24 }, get_font(0));
		}
	}

	return 0;
}