#pragma once
#include "SDL.h"
#include "Vec2D.h"
#include <vector>
#include <string>
#include <map>

using std::string;

//Wrapper for SDL keyboard input
class KeyboardInput
{
private:
	bool textmode = false;
	//const uint8_t* state = nullptr;
	std::vector<bool> state;
	std::vector<bool> last;
	std::vector<bool> temp;

public:
	string text_in;

	bool locked = false;
	bool numberInputOnly = false;

	KeyboardInput();
	~KeyboardInput() {}

	void update();
	void events(SDL_Event event);

	bool held(int key);
	bool pressed(int key);
	bool released(int key);

	//Start the collection of data typed as a string
	void openTextInput() { SDL_StartTextInput(); textmode = true; }
	void closeTextInput() { SDL_StopTextInput(); textmode = false; }
	string getTextInput();

	template<typename T> void	quickKeyboardControl(Vector2D<T>& pos, int force);
	template<typename T> void	quickKeyboardControlWASD(Vector2D<T>& pos, int force);
	void						quickKeyboardSelect(V2d_i& pos);
};

/**
 *	Wrapper for SDL mouse inputs
 *	Note: this class isn't perfect, when no SDL events are coming in, it might get stuck on a mouse event
		so sometimes pressed() might be true for more than a frame
*/
class MouseInput
{
private:
	Uint32 last = 0;
	Uint32 current = 0;
	SDL_Event event;
	int scrolly = 0;

public:
	bool locked = false;
	//must be called before mouse function are used
	void events(SDL_Event e);
	void update();

	bool held(int key); //0 = gauche 1 = droite 2 = millieu je pense
	bool pressed(int key); //le pressed ne marche pas, utiliser released
	bool released(int key);

	bool scrollup();
	bool scrolldown();
	int	 scroll();

	V2d_i position();
	V2d_i rpos(size_t ren = 0);

	void reset()
	{
		last = 0;
		current = 0;
	}
};

//Wrapper for SDL joystick inputs
//oui tu peut ajouter les controles avec une manette si tu veut
class JoystickInput
{
private:
	std::map<SDL_JoystickID, SDL_Joystick*> joysticks;
	std::map<SDL_JoystickID, std::map<int, bool>> buffers;
	std::map<SDL_JoystickID, std::map<int, bool>> last;

	/*
	* adds extra safeguards
	* returns false if device is NULL
	* or if the button is not within the joysticks' range
	*/
	bool held(SDL_Joystick* device, int button);
	bool pressed(SDL_Joystick* device, int button);
	bool released(SDL_Joystick* device, int button);

	int getAxisRaw(SDL_Joystick* device, int axis);

public:
	bool locked = false;
	int deadzone = 8000;

	JoystickInput();
	~JoystickInput();

	int getAxis(int player_index, int axis);

	//if player_index is -1, then it returns if any controller is pressing the button
	bool held(int player_index, int button);
	bool pressed(int player_index, int button);
	bool released(int player_index, int button);

	void addDevice(int device_index);
	void removeDevice(SDL_JoystickID instance_index);

	void events(SDL_Event event);
	void update();
};

template<typename T>
inline void KeyboardInput::quickKeyboardControl(Vector2D<T>& pos, int force)
{
	if (held(SDL_SCANCODE_UP))
		pos.y -= force;

	if (held(SDL_SCANCODE_DOWN))
		pos.y += force;

	if (held(SDL_SCANCODE_LEFT))
		pos.x -= force;

	if (held(SDL_SCANCODE_RIGHT))
		pos.x += force;
}

template<typename T>
inline void KeyboardInput::quickKeyboardControlWASD(Vector2D<T>& pos, int force)
{
	if (held(SDL_SCANCODE_W))
		pos.y -= force;

	if (held(SDL_SCANCODE_S))
		pos.y += force;

	if (held(SDL_SCANCODE_A))
		pos.x -= force;

	if (held(SDL_SCANCODE_D))
		pos.x += force;
}
