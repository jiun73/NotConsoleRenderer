#include "TileRenderer.h"
#include "Generics.h"
#include "ObjectGenerics.h"
#include "FunctionGenerics.h"
#include "ContainerGenerics.h"
#include "CstarParser.h"

void func() { std::cout << "Allo!" << std::endl; }
int number() { return 618; }
int add(int a, int b) { return a + b; }
void set(int& s) { s = 100; }

int main()
{
	int i = 69;
	GenericType<int> int_g(100);
	GenericType<int*> int_pg(&i);
	GenericFunctionType<function<void()>> func_g(func);
	GenericFunctionType<function<int()>> number_g(number);
	GenericFunctionType<function<int(int, int)>> add_g(add);
	GenericFunctionType<function<void(int&)>> set_g(set);
	GenericContainerType<vector<int>> vec_g({ 1,2,3,4 });
	GenericContainerType<map<string, int>> map_g({ {"no", 0},{"five", 5},{"second", 2},{"third", 3}});

	std::cout << int_g.stringify() << std::endl;
	std::cout << int_pg.stringify() << std::endl;
	std::cout << int_pg.dereference()->stringify() << std::endl;

	std::cout << int_g.type().name() << std::endl;
	std::cout << int_pg.type().name() << std::endl;
	std::cout << int_pg.dereference()->type().name() << std::endl;

	//int_g.set(make_generic(789));
	//int_pg.dereference()->set(make_generic(234));
	int_g.destringify("789");
	int_pg.dereference()->destringify("789");

	std::cout << int_g.stringify() << std::endl;
	std::cout << int_pg.stringify() << std::endl;
	std::cout << int_pg.dereference()->stringify() << std::endl;
	func_g.call();
	std::cout << number_g.type().name() << std::endl;
	std::cout << number_g.call()->stringify() << std::endl;

	add_g.args({ make_generic(1), make_generic(1) });
	std::cout << add_g.call()->stringify() << std::endl;

	/*set_g.args({ &int_g });
	set_g.call();
	std::cout << int_g.stringify() << std::endl;*/

	vec_g.insert(make_generic(200), 2);
	vec_g.erase(3);

	std::cout << vec_g.at(2)->stringify() << std::endl;
	std::cout << vec_g.container_size() << std::endl;
	std::cout << vec_g.stringify() << std::endl;

	std::cout << map_g.at(2)->stringify() << std::endl;
	std::cout << map_g.at(2)->type().name() << std::endl;

	CstarParser parser;

	File f("file.txt", FILE_READING_STRING);
	string source = f.getString();
	parser.parse(source);
}