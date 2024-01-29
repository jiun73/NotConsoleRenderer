#include "TileRenderer.h"
#include "EntityX.h"
#include "BasicSystems.h"
#include "ColliderSystem.h"
#include "Shape.h"

#include "Generics.h"
#include "ObjectGenerics.h"
#include "FunctionGenerics.h"
#include "ContainerGenerics.h"
#include "CstarParser.h"

void func() { std::cout << "Allo!" << std::endl; }
int number() { return 618; }
int add(int a, int b) { return a + b; }
void set(int& s) { s = 100; }


static ComponentXAdder<Shape_x> shape_adder;
static ComponentXAdder<Collider_x> coll_adder;

SystemXAdder<0, Shape_system, Position_x, Angle_x, Shape_x> shape_system_adder;
SystemXAdder<0, GFX_system, GFX_x, Shape_x> polygon_gfx_system_adder;
SystemXAdder<0, Controller_system, Position_x, Controller_x> controller_system_adder;
//SystemXAdder<1, Shooter_system, Shooter_x, Position_x, Angle_x> shooter_system_adder;
SystemXAdder<0, Physics_system, Physics_x, Position_x, Angle_x> physics_system_adder;
SystemXAdder<0, Lifetime_system, Lifetime_x> lifetime_system_adder;
SystemXAdder<1, Particle_system, ParticleEmitter_x, Position_x> particle_system_adder;
//SystemXAdder<0, PlayerMoveParticle_system, ParticleEmitter_x, Controller_x> player_particle_system_adder;
//SystemXAdder<1, Distorter_system, Distorter_x, Shape_x> distorter_system_adder;
//SystemXAdder<1, AI_system, AI_x, Angle_x, Position_x, Physics_x> ai_system_adder;
SystemXAdder<1, Health_system, Health_x> health_system_adder;
SystemXManagerAdder<0, Collision_system, Shape_x, Collider_x, Position_x> collision_system_adder;

enum Tags
{
	TAG_PLAYERS,
};

enum cTags
{
	SOLID = 1,
	PLAYER = 2
};

int main()
{
	TaggedEntityX<TAG_PLAYERS, Position_x, Angle_x, GFX_x, Shape_x, Controller_x, Collider_x> player;
	TaggedEntityX<TAG_PLAYERS, Position_x, Angle_x, GFX_x, Shape_x, Collider_x> floor;

	player.create({ 25 }, { 0 }, { rgb(100,100,100) }, { {-5, {5,-5}, 5, {-5,5}} }, {}, {SOLID});

	floor.create({ {0,40}, }, { 0 }, { rgb(100,100,100) }, { {0, {100,0}, {100,-5}, {0,-5}} }, { PLAYER });

	EntX::get()->get_system<Collision_system>()->add_pairing(PLAYER, SOLID, CTYPE_PUSH);
	//EntX::get()->get_system<Collision_system>()->add_pairing(SOLID, PLAYER, CTYPE_PUSH);

	set_window_size(100);
	set_window_resizable();

	while (run())
	{
		pencil(rgb(0, 0, 0));
		draw_clear();
	}

	//int i = 69;
	//shared_generic ptr = std::make_shared<GenericType<int>>();
	//GenericType<int> int_g(100);
	//GenericType<int*> int_pg(&i);
	//GenericFunctionType<function<void()>> func_g(func);
	//GenericFunctionType<function<int()>> number_g(number);
	//GenericFunctionType<function<int(int, int)>> add_g(add);
	//GenericFunctionType<function<void(int&)>> set_g(set);
	//GenericContainerType<vector<int>> vec_g({ 1,2,3,4 });
	//GenericContainerType<map<string, int>> map_g({ {"no", 0},{"five", 5},{"second", 2},{"third", 3}});

	//std::cout << int_g.stringify() << std::endl;
	//std::cout << int_pg.stringify() << std::endl;
	//std::cout << int_pg.dereference()->stringify() << std::endl;

	//std::cout << int_g.type().name() << std::endl;
	//std::cout << int_pg.type().name() << std::endl;
	//std::cout << int_pg.dereference()->type().name() << std::endl;

	////int_g.set(make_generic(789));
	////int_pg.dereference()->set(make_generic(234));
	//int_g.destringify("789");
	//int_pg.dereference()->destringify("789");

	//std::cout << int_g.stringify() << std::endl;
	//std::cout << int_pg.stringify() << std::endl;
	//std::cout << int_pg.dereference()->stringify() << std::endl;
	//func_g.call();
	//std::cout << number_g.type().name() << std::endl;
	//std::cout << number_g.call()->stringify() << std::endl;

	//add_g.args({ make_generic(1), make_generic(1) });
	//std::cout << add_g.call()->stringify() << std::endl;

	///*set_g.args({ &int_g });
	//set_g.call();
	//std::cout << int_g.stringify() << std::endl;*/

	//vec_g.insert(make_generic(200), 2);
	//vec_g.erase(3);

	//std::cout << vec_g.at(2)->stringify() << std::endl;
	//std::cout << vec_g.container_size() << std::endl;
	//std::cout << vec_g.stringify() << std::endl;

	//std::cout << map_g.at(2)->stringify() << std::endl;
	//std::cout << map_g.at(2)->type().name() << std::endl;

	//CstarParser parser;

	//File f("file.txt", FILE_READING_STRING);
	//string source = f.getString();
	//parser.parse(source);
}