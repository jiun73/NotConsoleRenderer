#include "TileRenderer.h"
#include "EntityX.h"
#include "Vec3D.h"
#include "Shape_x.h"
#include "Utility.h"
#include <functional>
#include "WeaponParser.h"
#include "BulletTypes.h"

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

struct Shooter_x 
{
	WeaponComputer computer;
	WeaponInventory inventory;
};

#include "BasicComponents.h"
#include "CollisionSystem.h"

ComponentXAdder<Shape_x> shape_adder;
ComponentXAdder<Collider_x> collider_adder;
ComponentXAdder<AI_x> ai_adder;
ComponentXAdder<Shooter_x> shooter_adder;

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

struct Shooter_system
{
	WeaponParser parser;

	void update(Shooter_x* shooter, Position_x* position, Angle_x* angle)
	{
		angle->angle = -getAngleTowardPoint(position->position, game_mouse_position()) + (0.5 * M_PI);

		if (mouse_left_held())
		{
			parser.load(shooter->computer, shooter->inventory);
			EntX::get()->add_callback_current([shooter](EntityID eid) {
				shooter->computer.tick(eid);
				}
			);
		}

		if (mouse_right_pressed())
		{
			parser.load(shooter->computer, shooter->inventory);
			shooter->computer.clockLast = SDL_GetTicks() - shooter->computer.settings.clockSpeed * 2 - shooter->computer.settings.bootSpeed;
			

			EntX::get()->add_callback_current([shooter](EntityID eid) {
					shooter->computer.tick(eid);
				}
			);
		}

		/*if (mouse().pressed(1))
		{
			EntX::get()->add_callback_current([position, angle](EntityID) {
				
				}
			);
		}*/

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

inline const int margin = 200;
inline const int window = 800;
inline const Rect left_border = { 0, {margin, window} };
inline const Rect right_border = { {margin + window,0}, {margin, window} };
inline const Rect main_window = { {margin, 0}, window };

enum Fonts 
{
	FONT_PIXEL
};

void render_intructions(const WeaponParser& parser, const Font& font, bool editing)
{
	size_t i = 0;
	for (auto& lines : parser.source)
	{
		if (parser.computer->instructions.at(i).data)
			set_font_pencil(COLOR_GREEN, font);
		else
			if (editing)
				set_font_pencil(COLOR_WHITE, font);
			else
				set_font_pencil(rgb(100, 100, 100), font);

		draw_text(lines, 800, { 0, (int)i * font.height }, font);
		i++;
	}
}

void change_line(const WeaponParser& parser, int& current_line, int n)
{
	current_line = n;
	current_line = std::max(current_line, 0);
	current_line = std::min(current_line, parser.computer->settings.storage - 1);
	keyboard().getTextInput() = parser.source.at(current_line);
}

void render_registers(const WeaponParser& parser, const Font& font)
{
	for (size_t i = 0; i < parser.computer->registers.size(); i++)
	{
		WeaponRegister& reg = parser.computer->registers.at(i);
		char c = 'A' + (char)i;
		string s = "  " + std::to_string(reg.bit - reg._signed_) + "-bit";
		s.at(0) = c;

		string ss = "" + s + ": " + std::to_string(parser.computer->registers.at(i).value);
		draw_text(ss, window, { 0,window / 2 + (int)i * font.height }, font);
	}

	size_t i = 0;
	for (auto& reg_pair : parser.computer->bulletType.registers)
	{
		string s = reg_pair.first + ":  " + std::to_string(reg_pair.second.value);
		draw_text(s, window, { left_border.sz.x / 2,window / 2 + (int)i * font.height }, font);
		i++;
	}
}

void load_sounds()
{
	sound().loadSound("Sounds/shoot_sfx1.wav");
	sound().loadSound("Sounds/shoot_sfx2.wav");
	sound().loadSound("Sounds/shoot_sfx3.wav");
	sound().loadSound("Sounds/shoot_sfx4.wav");
}

void set_collision_pairs() 
{
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_TESTOBJ, TAG_PLAYER_ | TAG_TESTOBJ | TAG_ENEMY__, CTYPE_PUSH);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_TESTOBJ, TAG_PBULLET | TAG_EBULLET, CTYPE_DESTROY);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_PBULLET, TAG_ENEMY__, CTYPE_HURT);
//	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_EBULLET, TAG_PLAYER_, CTYPE_HURT);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_ENEMY__, TAG_PBULLET, CTYPE_DESTROY);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_ENEMY__, TAG_ENEMY__, CTYPE_PUSH);
	EntX::get()->get_system<Collision_system>()->add_pairing(TAG_PLAYER_, TAG_EBULLET, CTYPE_DESTROY);
}

void render_errors(vector<WeaponError>& errors, const Font& font) 
{
	pencil(COLOR_PINK);

	for (auto& error : errors)
	{
		string line_text = keyboard().getTextInput();
		int text_size;
		char character;
		error.offset -= 2;

		if (line_text.size() > error.offset)
		{
			text_size = hidden::get_text_draw_size(line_text.cbegin(), line_text.cbegin() + error.offset, font);
			character = line_text.at(error.offset);
		}
		else
		{
			text_size = 0;
			character = 'a';
		}

		Rect dest;
		dest.pos = { text_size, error.line * font.height };
		dest.sz = { font.get(character).advance,font.height };
		draw_rect(dest);

		draw_simple_text(get_error_message(error), { margin, error.line * font.height }, font);
	}
}

void render_line_selection(const WeaponParser& parser, const Font& font) 
{
	for (size_t i = 0; i < parser.source.size(); i++)
	{
		Rect dest;
		dest.pos = { 0, font.height * (int)i };
		dest.sz = { margin, font.height };

		if (point_in_rectangle(mouse_position(), dest))
		{
			pencil(COLOR_WHITE);
			draw_rect(dest);
		}
		
	}
}

void handle_line_selection(const WeaponParser& parser, const Font& font, int& selected_line) 
{
	for (size_t i = 0; i < parser.source.size(); i++)
	{
		Rect dest;
		dest.pos = { 0, font.height * (int)i };
		dest.sz = { margin, font.height };

		if (point_in_rectangle(mouse_position(), dest) && mouse_left_pressed()) change_line(parser, selected_line, (int)i);
	}
}

void render_cursor(const string& input, const int& selected_line, const Font& font)
{
	Rect dest;
	dest.pos = { get_text_draw_size(input, font), selected_line * font.height };
	dest.sz = { font.get('A').sdl_dest.w, font.get('A').sdl_dest.h };
	pencil(COLOR_WHITE);
	draw_full_rect(dest);
}

void handle_new_line_insert(WeaponParser& parser, const int& selected_line)
{
	if (parser.source.back() == "")
	{
		parser.source.insert(parser.source.begin() + selected_line, "");
		size_t i = selected_line + 1;
		for (auto it = parser.source.begin() + selected_line + 1; it != parser.source.end(); it++) //ensure that data intruction don't change addresses
		{
			if (i >= parser.computer->settings.storage) break;
			if (parser.computer->instructions.at(i - 1).data)
				std::swap(parser.source.at(i), parser.source.at(i - 1));
			i++;
		}
		keyboard().getTextInput() = parser.source.at(selected_line);
		parser.source.pop_back();
	}
}

void handle_edit_mode(WeaponParser& parser, int& selected_line, const Font& font)
{
	keyboard().openTextInput();
	string& input = keyboard().getTextInput();
	parser.source.at(selected_line) = input;

	if (key_pressed(SDL_SCANCODE_RETURN))
	{
		input.pop_back();
		parser.source.at(selected_line) = input;
		change_line(parser, selected_line, selected_line + 1);
		handle_new_line_insert(parser, selected_line);
	}

	static int inputWasEmpty = false;
	int lineMove = 0;

	if (key_pressed(SDL_SCANCODE_BACKSPACE) && input.empty() && inputWasEmpty) lineMove = -1;
	if (key_pressed(SDL_SCANCODE_UP)) lineMove = -1;
	if (key_pressed(SDL_SCANCODE_DOWN)) lineMove = 1;

	if(lineMove) change_line(parser, selected_line, selected_line + lineMove);
	inputWasEmpty = input.empty();

	auto errors = parser.load(*parser.computer, *parser.inventory);

	render_cursor(input, selected_line, font);
	render_errors(errors, font);
	render_line_selection(parser, font);
	handle_line_selection(parser, font, selected_line);
}

void render_program_counter(const WeaponParser& parser, const Font& font)
{
	Rect dest;
	dest.pos = { 0, font.height * ((int)(parser.computer->programCounter)) };
	dest.sz = { margin, font.height };
	pencil(rgb(50, 50, 50));
	draw_full_rect(dest);
}

int main()
{
	set_window_size({800 + margin * 2,800});
	set_window_resizable();

	init();

	load_sounds();
	set_collision_pairs();

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

	WeaponComputer& computer = player.component<Shooter_x>()->computer;
	WeaponInventory& inventory = player.component<Shooter_x>()->inventory;
	computer.bulletType = get_bullet_from_type(BULLET_SIZABLE);
	computer.settings.storage = 25;
	computer.settings.clockSpeed = 50;
	computer.registers.resize(3);
	inventory.available_intructions = { {SHOOT, {2} },{LOAD, {2} } };

	GFX_system* gfx_sys = EntX::get()->get_system<GFX_system>();
	WeaponParser& parser = EntX::get()->get_system<Shooter_system>()->parser;
	parser.load(computer, inventory);
	
	bool edit = true;
	bool inputWasEmpty = false;
	int selected_line = 0;

	const Font& pixel_font = get_font(FONT_PIXEL);

	keyboard().lockInputsInTextmode = false;
	keyboard().convInputUpper = true;

	while (run())
	{
		//global_drawing_offset() = 0;

		pencil(COLOR_CYAN);
		draw_clear();

		pencil(COLOR_BLACK);
		draw_full_rect({ 0, {margin,window} });
		draw_full_rect({ {window + margin, 0}, {margin,window} });

		render_program_counter(parser, pixel_font);
		render_intructions(parser, pixel_font, edit);
		render_registers(parser, pixel_font);

		string status_text;
		string temp_text = std::to_string(computer.temperature * 10.0 + 22.0) + " C";

		if (computer.overheat)
			status_text = "Overheating!";
		else if (computer.booting)
			status_text = "Status: booting";
		else
			status_text = "Status: working";

		draw_text(temp_text, 800, left_border.pos + V2d_i(0, 200), pixel_font);
		draw_text(status_text, 800, left_border.pos + V2d_i(0, 200 + pixel_font.height), pixel_font);

		if (edit)
		{
			handle_edit_mode(parser, selected_line, pixel_font);

			if (mouse_left_pressed() && point_in_rectangle(mouse_position(), main_window ))
			{
				std::cout << "exit edit mode" << std::endl;
				keyboard().closeTextInput();
				edit = false;
			}
		}
		else if (mouse_left_pressed() && point_in_rectangle(mouse_position(), left_border))
		{
			std::cout << "enter edit mode" << std::endl;
			edit = true;
		}

		draw_special_text("bru\\wave.\\rainbow.uuuuuuuuhhhhh", 200, 100, pixel_font);

		camera().offset = player.component<Position_x>()->position - get_window_size() / 2;
	}

	return 0;
}