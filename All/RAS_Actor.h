#pragma once
#include <map>
#include <functional>

namespace RAS {
	typedef size_t Time;
	typedef size_t GeneratorID;
	using std::map;
	using std::vector;
	using std::function;
	 
	struct Manager;

	//A function that changes data according to the time
	struct Modifier 
	{
		
	};

	//Sequence of modifiers, describing the evolution in time of a data field according to external input
	struct Event
	{
		Manager* manager;
		GeneratorID generator;
	};

	//Generates Events of a certain type 
	struct Generator 
	{

	};

	//Set of events for an Actor
	struct Timeline
	{
		map<Time, Event> events;
	};

	struct Actor
	{
		Timeline timeline;
	};

	//Handles semi-deterministic events, like collision detection
	struct System
	{

	};

	//Interface for the developper
	struct Manager
	{
		vector<Actor> actors;
		vector<System> systems;
		vector<Generator> generators;
	};
}