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
	CONTROLLER_INPUT
};

enum InputActionType
{
	INPUT_HELD,
	INPUT_PRESSED,
	INPUT_RELEASED,
	INPUT_AXIS_NEGATIVE,
	INPUT_AXIS_POSITIVE,
};

struct MappedInput
{
	InputType type;
	InputActionType action;
	int button_index;
	int player_index = 0; //for joysticks
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
	std::map<std::string, std::pair<std::list<MappedInput>, bool>> mapped_inputs;

	void handleMouseMap(MappedInput& key, bool& found);
	void handleKeyboardMap(MappedInput& key, bool& found);
	void handleControllerMap(MappedInput& key, bool& found);

	bool input_locked = false;
	uint32_t input_key = 0;

	uint32_t lock_inputs()
	{
		uint32_t key = SDL_GetTicks();
		input_key = key;
		input_locked = true;
		return key;
	}

	void unlock_inputs(uint32_t key)
	{
		if (input_key == key)
			input_locked = false;
	}

public:
	MouseInput mouse;
	KeyboardInput keyboard;
	JoystickInput joystick;

	void loadMappings(std::string path);
	void saveCurrentMapping(std::string path);

	void map(std::string name, std::initializer_list<MappedInput> map);
	void map(std::string name, MappedInput map);

	bool check(const std::string& name);
	void events(SDL_Event event);
	void update();
	void update_locks();

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

	static KeyboardInput& keyboard() { get()->update_locks();  return get()->keyboard; }
	static MouseInput& mouse() { get()->update_locks(); return get()->mouse; }
	static JoystickInput& joystick() { get()->update_locks(); return get()->joystick; }
	static bool				map(const string& input) { return get()->check(input); }

};

struct InputLock
{
	bool locked = false;
	uint32_t key = 0;

	InputLock() {};
	~InputLock() { unlock(); };

	void lock() { key = Controls::get()->lock_inputs(); locked = true; }
	void unlock() { if (locked) Controls::get()->unlock_inputs(key); }
};
