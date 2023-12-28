#include "Input.h"
#include "pch.h"

//must be called before mouse function are used
void MouseInput::events(SDL_Event e)
{
	/*for (size_t i = 0; i < 4; i++)
		last_mouse_state[i] = current_mouse_state[i];*/

		/*event = e;
		for (size_t i = 0; i < 4; i++)
			if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == i)
				current_mouse_state[i] = true;
			else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == i)
				current_mouse_state[i] = false;*/

	if (event.type == SDL_MOUSEWHEEL)
		scrolly = event.wheel.y;
}

void MouseInput::update()
{
	last = current;
	current = SDL_GetMouseState(NULL, NULL);
}

bool MouseInput::scrollup()
{
	if (event.type == SDL_MOUSEWHEEL)
		if (event.wheel.y > 0)
			return true;
	return false;
}

bool MouseInput::scrolldown()
{
	if (event.type == SDL_MOUSEWHEEL)
		if (event.wheel.y < 0)
			return true;
	return false;
}

int MouseInput::scroll() { return scrolly; }

//check if a key is being held

bool MouseInput::held(int key)
{
	return (current & SDL_BUTTON(key)) && !locked;
	//return current_mouse_state[key] && !locked;
	/*if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == key)
	return true;
	return false;
	*/
}

//check if a key is being pressed

bool MouseInput::pressed(int key)
{
	return held(key) && !(last & SDL_BUTTON(key)) && !locked;
	//return (current_mouse_state[key] && !last_mouse_state[key]) && !locked;
}

V2d_i MouseInput::position()
{
	int x, y;
	SDL_GetMouseState(&x, &y);
	return V2d_i(x, y);
}

//V2d_i MouseInput::rpos(size_t ren) {
//	return Rendering::get(ren)->getScreenPosition(position()) - Rendering::get(ren)->getWiewPos();
//}

//check if a key is released

bool MouseInput::released(int key)
{
	return !held(key) && (last & SDL_BUTTON(key)) && !locked;
	//return (!current_mouse_state[key] && last_mouse_state[key]);
}

KeyboardInput::KeyboardInput()
{
	int numkeys;
	const uint8_t* s = SDL_GetKeyboardState(&numkeys);
	for (size_t i = 0; i < numkeys; i++)
	{
		state.push_back(false);
		last.push_back(false);
		temp.push_back(false);
	}
}

void KeyboardInput::update()
{
	if (!textmode)
	{
		last = state;
		int numkeys;
		const uint8_t* s = SDL_GetKeyboardState(&numkeys);

		state.clear();

		for (size_t i = 0; i < numkeys; i++)
		{
			state.push_back(s[i]);
		}
	}
	else
	{

	}
}

void KeyboardInput::events(SDL_Event e)
{
	if (e.type == SDL_KEYDOWN)
	{
		if (e.key.keysym.sym == SDLK_BACKSPACE && text_in.length() > 0)				//Handle backspace
			text_in.pop_back();
		else if (e.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL)				//Handle copy
			SDL_SetClipboardText(text_in.c_str());
		else if (e.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL)				//Handle paste
			text_in = SDL_GetClipboardText();
		else if (e.key.keysym.sym == SDLK_RETURN)	//Handle paste
		{
			text_in += "\n";
		}
	}
	else if (e.type == SDL_TEXTINPUT)
	{
		//Not copy or pasting
		if (!(SDL_GetModState() & KMOD_CTRL && (e.text.text[0] == 'c' || e.text.text[0] == 'C' || e.text.text[0] == 'v' || e.text.text[0] == 'V')))
		{
			//Append character
			if (!numberInputOnly)
			{
				text_in += e.text.text;
			}
			else
			{
				int i = *e.text.text;
				if (i >= 48 && i <= 57) //only if is a number
					text_in += e.text.text;

				if (text_in.empty() && i == 45) //allows '-' to be written if the text is empty (for negative numbers)
					text_in += e.text.text;
			}
		}
	}
}

bool KeyboardInput::held(int key)
{
	return state[key] && !textmode;
}

bool KeyboardInput::pressed(int key)
{
	return (state[key] && !last[key]);
}

bool KeyboardInput::released(int key)
{

	return (!state[key] && last[key]);
}

void KeyboardInput::quickKeyboardSelect(V2d_i& pos)
{
	if (pressed(SDL_SCANCODE_W))
		pos.y -= 1;

	if (pressed(SDL_SCANCODE_S))
		pos.y += 1;

	if (pressed(SDL_SCANCODE_A))
		pos.x -= 1;

	if (pressed(SDL_SCANCODE_D))
		pos.x += 1;
}

string KeyboardInput::getTextInput()
{
	return text_in;
}

/*
* adds extra safeguards
* /returns false if device is NULL
* /or if the button is not within the joysticks' range
*/
bool JoystickInput::held(SDL_Joystick* device, int button)
{
	if (button >= SDL_JoystickNumButtons(device) || device == NULL)
		return false;

	return SDL_JoystickGetButton(device, button) && !locked;
}

bool JoystickInput::pressed(SDL_Joystick* device, int button)
{
	SDL_JoystickID id = SDL_JoystickInstanceID(device);
	bool bheld = held(device, button);

	if (bheld && !last[id][button])
	{
		buffers[id][button] = true;
		return true;
	}
	else if (!bheld)
		buffers[id][button] = false;

	return false;
}

bool JoystickInput::released(SDL_Joystick* device, int button)
{
	SDL_JoystickID id = SDL_JoystickInstanceID(device);
	bool bheld = held(device, button);

	if (!bheld && last[id][button])
	{
		buffers[id][button] = false;
		return true;
	}
	else if (bheld)
		buffers[id][button] = true;

	return false;
}

int JoystickInput::getAxisRaw(SDL_Joystick* device, int axis)
{
	if (axis >= SDL_JoystickNumAxes(device) || device == NULL)
		return 0;

	return SDL_JoystickGetAxis(device, axis);
}

JoystickInput::JoystickInput()
{
	int numjoy = SDL_NumJoysticks();
	if (numjoy > 0)
		for (int i = 0; i < numjoy; i++) //open all the already connectted joysticks
			addDevice(i);

	/*DefineCommand cmd("/controls joystick num", [&](__COMMAND_ARGS__)
		{
			std::cout << joysticks.size() << std::endl;
		});*/
}

JoystickInput::~JoystickInput()
{
	std::cout << "Joystick input shutting down..." << std::endl;
	for (auto joy : joysticks)
		removeDevice(joy.first);
}

int JoystickInput::getAxis(int player_index, int axis)
{
	int value = getAxisRaw(SDL_JoystickFromPlayerIndex(player_index), axis);

	int sign = 0;
	if (value > 0)
		sign = 1;
	else
		sign = -1;

	return (abs(value) > deadzone) * sign;
}

//if player_index is -1, then it returns if any controller is pressing the button
bool JoystickInput::held(int player_index, int button)
{
	if (player_index >= 0)
		return held(SDL_JoystickFromPlayerIndex(player_index), button);

	for (auto joy : joysticks)
		if (held(joy.second, button))
			return true;

	return false;
}

bool JoystickInput::pressed(int player_index, int button)
{
	if (player_index >= 0)
		return pressed(SDL_JoystickFromPlayerIndex(player_index), button);

	for (auto joy : joysticks)
		if (pressed(joy.second, button))
			return true;

	return false;
}

bool JoystickInput::released(int player_index, int button)
{
	if (player_index >= 0)
		return released(SDL_JoystickFromPlayerIndex(player_index), button);

	for (auto joy : joysticks)
		if (released(joy.second, button))
			return true;

	return false;
}

void JoystickInput::addDevice(int device_index)
{
	SDL_Joystick* device = SDL_JoystickOpen(device_index);
	SDL_JoystickID id = SDL_JoystickGetDeviceInstanceID(device_index);
	std::cout << SDL_JoystickName(device) << " connected as player " << SDL_JoystickGetPlayerIndex(device) << std::endl;

	joysticks.emplace(id, device);
}

void JoystickInput::removeDevice(SDL_JoystickID instance_index)
{
	SDL_JoystickID id = instance_index;
	SDL_Joystick* device = joysticks[id];
	std::cout << SDL_JoystickName(device) << " disconnected" << std::endl;

	SDL_JoystickClose(device);
	joysticks.erase(id);
}

void JoystickInput::events(SDL_Event event)
{
	if (event.type == SDL_JOYDEVICEADDED)
		addDevice(event.jdevice.which);

	if (event.type == SDL_JOYDEVICEREMOVED)
		removeDevice(event.jdevice.which);
}

void JoystickInput::update()
{
	last = buffers;
}
