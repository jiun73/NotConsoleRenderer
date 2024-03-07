#pragma once
#include "WeaponParser.h"
#include "EntityX.h"
#include "BasicComponents.h"
#include "CollisionComponent.h"

enum BulletTypes 
{
	BULLET_SIZABLE,
};

inline WeaponBullet get_bullet_from_type(BulletTypes type) 
{
	switch (type)
	{
	case BULLET_SIZABLE:
		return { { {"SIZE", {}}}, [](WeaponBullet& b, size_t playerid)
			{
				Position_x* pos = ECSX::EntX::get()->get_entity_component<Position_x>(playerid);
				Angle_x* angle = ECSX::EntX::get()->get_entity_component<Angle_x>(playerid);

				ECSX::EntityX<Position_x, Angle_x, GFX_x, Shape_x, Physics_x, Lifetime_x, Collider_x> bullet;
				WeaponRegister& size_reg = b.registers.at("SIZE");
				int size = size_reg.value + 1;
				size_reg.value = 0;

				bullet.create(
					*pos,
					{ random().frange(-2 * M_PI, 2 * M_PI) },
					{ COLOR_WHITE },
					{ { {-5.0 * size,5.0 * size},{10.0 * size,0.0 * size}, -5.0 * size } },
					{ V2d_d().polar(5, angle->angle), 0, 0 },
					{ 100 },
					{ TAG_PBULLET }
				);

				sound().playSound("Sounds/shoot_sfx" + std::to_string(random().range(1, 4)) + ".wav");
				std::cout << "pew!" << std::endl;
			} };
	default:
		break;
	}

	return {};
}