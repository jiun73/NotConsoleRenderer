#pragma once
#include "RAS.h"
#include "FMods.h"

namespace FF
{
	enum Gens
	{
		SET_STATIC,
		SET_VELOCITY,
		SET_ACCELLERATION,
		STAY
	};

	void add_generators(RAS::Manager& manager)
	{
		manager.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
			return RAS::Event().add_modifier<Mvt, 1>(0, mvt_static, { (double)extra.at(0) });
			}), SET_STATIC);

		manager.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
			Mvt old = manager->actor_field_at<Mvt>(time - 1, actor, generator_field);
			return RAS::Event().add_modifier<Mvt, 2>(0, mvt_linear, { old.pos, (double)extra.at(0) });
			}), SET_VELOCITY);

		manager.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
			Mvt old = manager->actor_field_at<Mvt>(time - 1, actor, generator_field);
			return RAS::Event().add_modifier<Mvt, 3>(0, mvt_quad, { old.pos, old.vel, (double)extra.at(0) });
			}), SET_ACCELLERATION);
		
		manager.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
			Mvt old = manager->actor_field_at<Mvt>(time - 1, actor, generator_field);
			return RAS::Event().add_modifier<Mvt, 1>(0, mvt_static, { old.pos });
			}), STAY);

	}
}