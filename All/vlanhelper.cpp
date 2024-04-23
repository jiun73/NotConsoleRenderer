#include "pch.h"

const FactoryManagerAdder<vector<string>>* vector_animf__adder = new FactoryManagerAdder<vector<string>>("vector(string)", true);

void vlan_helper() 
{
	GLUU::Compiled_ptr menu = GLUU::parse_file("vlanhelper.gluu");

	while (run())
	{
		pencil(COLOR_CYAN);
		draw_clear();

		menu->render({ 0,get_logical_size() });
	}
}

namespace VLAN
{
	GLUU_IMPORT_MAIN(vlan_helper);
}
