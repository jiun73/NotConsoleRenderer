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
		SET_ACCELLARATION_SNAP_ZERO,
		DECCELLARATION,
		STAY
	};

	RAS::Event set_acceleration_snap_zero(Mvt old, double a)
	{
		if ((a > 0 && old.vel > 0) || (a < 0 && old.vel < 0))
		{
			return RAS::Event()
				.add_modifier<Mvt, 3>(0, mvt_quad, { old.pos, old.vel, a });
		}

		RAS::Time tstop = linear_inverse(0, { old.vel, a });
		Mvt stopmvt;
		mvt_quad(tstop, stopmvt, { old.pos, old.vel, a });

		return RAS::Event()
			.add_modifier<Mvt, 3>(0, mvt_quad, { old.pos, old.vel, a })
			.add_modifier<Mvt, 1>(tstop, mvt_static, { stopmvt.pos });
	}

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

		manager.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
			Mvt old = manager->actor_field_at<Mvt>(time - 1, actor, generator_field);
			return set_acceleration_snap_zero(old, extra.at(0));
			}), SET_ACCELLARATION_SNAP_ZERO);

		manager.register_generator(RAS::Generator([](GENERATOR_ARGS) -> RAS::Event {
			Mvt old = manager->actor_field_at<Mvt>(time - 1, actor, generator_field);
			double a = abs(extra.at(0));
			if (old.vel > 0)
			{
				a = -a;
			}
			else if (old.vel == 0)
			{
				return RAS::Event();
			}

			return set_acceleration_snap_zero(old, a);
			}), DECCELLARATION);

	}
}