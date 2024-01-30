#include "Controls.h"
#include "pch.h"

MultiInput* Controls::min;

void MultiInput::handleMouseMap(MappedInput& key, bool& found)
{
	switch (key.action)
	{
	case INPUT_PRESSED:
		if (mouse.pressed(key.button_index))
			found = true;
		return;
	case INPUT_HELD:
		if (mouse.held(key.button_index))
			found = true;
		return;
	}
}

void MultiInput::handleKeyboardMap(MappedInput& key, bool& found)
{
	switch (key.action)
	{
	case INPUT_PRESSED:

		if (keyboard.pressed(key.button_index))
			found = true;
		return;
	case INPUT_HELD:
		if (keyboard.held(key.button_index))
			found = true;
		return;
	case INPUT_RELEASED:
		if (keyboard.released(key.button_index))
			found = true;
		return;
	}
}

void MultiInput::handleControllerMap(MappedInput& key, bool& found)
{
	switch (key.action)
	{
	case INPUT_HELD:
		if (joystick.held(key.player_index, key.button_index))
			found = true;
		return;
	case INPUT_PRESSED:
		if (joystick.pressed(key.player_index, key.button_index))
			found = true;
		return;
	case INPUT_RELEASED:
		if (joystick.released(key.player_index, key.button_index))
			found = true;
		return;
	case INPUT_AXIS_POSITIVE:
		if (joystick.getAxis(key.player_index, key.button_index) == 1)
			found = true;
		return;
	case INPUT_AXIS_NEGATIVE:
		if (joystick.getAxis(key.player_index, key.button_index) == -1)
			found = true;
		return;
	}
}

void MultiInput::loadMappings(std::string path)
{
	/*File f(path);
	f.open_read();

	size_t ksz = f.read<size_t>();
	std::cout << "a" << std::endl;
	for (size_t i = 0; i < ksz; i++)
	{
		std::string name;
		name = f.readwrite_string(name, "name");
		std::list<MappedInput> list;
		size_t msz = f.read<size_t>();
		for (size_t i = 0; i < msz; i++)
			list.emplace_back(f.read<MappedInput>());
		mapped_inputs.emplace(name, std::pair<std::list<MappedInput>, bool>(list, false));
	}

	std::cout << "Loaded input config from '" << path << "'" << std::endl;*/
}

void MultiInput::saveCurrentMapping(std::string path)
{/*
	File f(path);
	f.open_write();

	f.write<size_t>(mapped_inputs.size());
	for (auto& map : mapped_inputs)
	{
		f.readwrite_string(map.first, "name");
		f.write<size_t>(map.second.first.size());
		for (auto& key : map.second.first)
			f.write(key);
	}

	std::cout << "Saved current input config as '" << path << "'" << std::endl;*/
}

void MultiInput::map(std::string name, std::initializer_list<MappedInput> map)
{
	std::list<MappedInput> list = map;
	mapped_inputs.emplace(name, std::make_pair(list, false));
}

void MultiInput::map(std::string name, MappedInput map)
{
	std::list<MappedInput> list = { map };
	mapped_inputs.emplace(name, std::make_pair(list, false));
}

bool MultiInput::check(const std::string& name)
{
	return mapped_inputs.at(name).second && !input_locked;
}

void MultiInput::events(SDL_Event event)
{
	keyboard.events(event);
	mouse.events(event);
	joystick.events(event);
}

void MultiInput::update()
{
	update_locks();

	mouse.update();
	//keyboard.update();
	joystick.update();

	for (auto& k : mapped_inputs)
	{
		std::list<MappedInput>& keys = k.second.first;

		bool found = false;
		for (auto& key : keys)
		{
			switch (key.type)
			{
			case MOUSE_INPUT:
				handleMouseMap(key, found);
				break;
			case KEYBOARD_INPUT:
				handleKeyboardMap(key, found);
				break;
			case CONTROLLER_INPUT:
				handleControllerMap(key, found);
				break;
			default:
				break;
			}

			if (found)
				break;
		}

		k.second.second = found;
	}
}

void MultiInput::update_locks()
{
	keyboard.locked = input_locked;
	mouse.locked = input_locked;
	joystick.locked = input_locked;
}
