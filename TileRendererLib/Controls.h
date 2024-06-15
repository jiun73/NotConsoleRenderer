#pragma once
#include "Input.h"
#include <map>
#include <list>
#include "SDL.h"
//#include "FunctionsExtended.h"
//#include "FileInterpret.h"

enum InputType
{
	MOUSE_INPUT,
	KEYBOARD_INPUT,
	CONTROLLER_INPUT,
	CONTROLLER_AXISPOSITIVE_INPUT,
	CONTROLLER_AXISNEGATIVE_INPUT,
};

enum InputActionType
{
	INPUT_HELD,
	INPUT_PRESSED,
	INPUT_RELEASED,
};

struct MappedInput
{
	InputType type;
	int button_index;
	int player_index = -1; //for joysticks
};

struct MappedInputList
{
	std::list<MappedInput> possible_inputs_source;
	bool current_state = false;
	bool old_state = false;

	MappedInputList() {}
	MappedInputList(std::list<MappedInput> list) : possible_inputs_source(list) {}
	~MappedInputList() {}
};

struct InputLock;

/*
* Class that combines Joystick, Mouse and keyboard input
* It allows to map input names to different input
* ex: "jump" could be mapped to a W press AND a A (joystick) press
*/
class MultiInput
{
private:
	std::map<std::string, MappedInputList> mapped_inputs;

	bool single_mapping_state(const MappedInput& input)
	{
		switch (input.type)
		{
		case KEYBOARD_INPUT:
			return keyboard.held(input.button_index);

		case MOUSE_INPUT:
			return mouse.held(input.button_index);

		case CONTROLLER_INPUT:
			return joystick.held(input.player_index, input.button_index);

		case CONTROLLER_AXISPOSITIVE_INPUT:
		{
			int axis_value = joystick.getAxis(input.player_index, input.button_index);
			return (axis_value > 0);
		}

		case CONTROLLER_AXISNEGATIVE_INPUT:
		{
			int axis_value = joystick.getAxis(input.player_index, input.button_index);
			return (axis_value < 0);
		}

		default:
			break;
		}
	}

	bool full_mapping_state(const MappedInputList& list) 
	{
		bool state = false;
		for (auto i : list.possible_inputs_source)
		{
			if (single_mapping_state(i))
			{
				state = true;
				break;
			}
		}

		return state;
	}

public:
	MouseInput mouse;
	KeyboardInput keyboard;
	JoystickInput joystick;

	void loadMappings(std::string path);
	void saveCurrentMapping(std::string path);

	void map(std::string name, std::initializer_list<MappedInput> map);
	void map(std::string name, MappedInput map);

	bool check(const std::string& name, InputActionType action);
	void events(SDL_Event event);
	void update();

	friend InputLock;
};

class Controls //MultiInput singleton
{
private:
	static MultiInput* min;

public:
	static MultiInput* get()
	{
		if (min == nullptr)
			min = new MultiInput();
		return min;
	}

	static KeyboardInput& keyboard() { return get()->keyboard; }
	static MouseInput& mouse() { return get()->mouse; }
	static JoystickInput& joystick() { return get()->joystick; }
	//static bool				map(const string& input) { return get()->check(input); }

};

//struct InputLock
//{
//	bool locked = false;
//	uint32_t key = 0;
//
//	InputLock() {};
//	~InputLock() { unlock(); };
//
//	void lock() { key = Controls::get()->lock_inputs(); locked = true; }
//	void unlock() { if (locked) Controls::get()->unlock_inputs(key); }
//};
