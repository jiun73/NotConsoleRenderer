#pragma once
#include "RAS.h"
#include "FunctionMod.h"

namespace FF
{
	enum Gens 
	{
		SET_STATIC
	};

	void add_generators(RAS::Manager& manager)
	{
		manager.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
			return RAS::Event().add_modifier<double, 1>(0, static_func<double>, STATIC, { (double)extra.at(0) });
			}), SET_STATIC);
	}
}