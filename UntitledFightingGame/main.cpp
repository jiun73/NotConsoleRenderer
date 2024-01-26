#include "TileRenderer.h"
#include "Generics.h"

int main()
{
	int i = 69;
	GenericType<int> int_g(100);
	GenericType<int*> int_pg(&i);

	std::cout << int_g.stringify() << std::endl;
	std::cout << int_pg.stringify() << std::endl;
	std::cout << int_pg.dereference()->stringify() << std::endl;

	std::cout << int_g.type().name() << std::endl;
	std::cout << int_pg.type().name() << std::endl;
	std::cout << int_pg.dereference()->type().name() << std::endl;

	int_g.set(make_generic(789));
	int_pg.dereference()->set(make_generic(234));

	std::cout << int_g.stringify() << std::endl;
	std::cout << int_pg.stringify() << std::endl;
	std::cout << int_pg.dereference()->stringify() << std::endl;
}