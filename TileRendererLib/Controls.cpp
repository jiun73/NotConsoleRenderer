#include "Controls.h"
#include "pch.h"

MultiInput* Controls::min;

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
	mapped_inputs.emplace(name, MappedInputList(list) );
}

void MultiInput::map(std::string name, MappedInput map)
{
	std::list<MappedInput> list = { map };
	mapped_inputs.emplace(name, MappedInputList(list));
}

bool MultiInput::check(const std::string& name, InputActionType action)
{
	switch (action)
	{
	case INPUT_HELD:
		return mapped_inputs.at(name).current_state;
	case INPUT_PRESSED:
		return mapped_inputs.at(name).current_state && !mapped_inputs.at(name).old_state;
	case INPUT_RELEASED:
		return !mapped_inputs.at(name).current_state && mapped_inputs.at(name).old_state;
	default:
		return 0;
	}
}

void MultiInput::events(SDL_Event event)
{
	keyboard.events(event);
	mouse.events(event);
	joystick.events(event);
}

void MultiInput::update()
{
	mouse.update();
	//keyboard.update();
	joystick.update();

	for (auto& k : mapped_inputs)
	{
		k.second.old_state = k.second.current_state;
		k.second.current_state = full_mapping_state(k.second);
	}
}
