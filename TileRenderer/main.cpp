
#include "TileRenderer.h"

/*
* Ceci est un exemple!
* Tu peut regarder dans TileRenderer.h, il y a d'autres fonctions 
* (par exemple, tu peut dessiner des images... :) )
* 
* Ya pas encore de texte, j'imagine tu peut utiliser la console pour ca 
* ou si tu est plus ambitieux, tu peut essayer de le faire toi même
* 
* Juste a creer un nouveau proet dans la solution et copier les settings de 'exemple'
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