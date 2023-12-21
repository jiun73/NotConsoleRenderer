
#include "TileRenderer.h"

/*
* Ceci est un exemple!
* Tu peut regarder dans TileRenderer.h, il y a d'autres fonctions
*/

int main() 
{
	set_window_size(200);
	set_window_resizable();

	while (run()) 
	{
		pencil(rgb(255, 0, 0));
		draw_line(10, 20);

		if (mouse().pressed(1))
		{
			std::cout << "bruh" << std::endl;
		}
	}

	return 0;
}