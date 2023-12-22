
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
		pencil(rgb(255, 255, 0));
		
		draw_line({ 10,0 }, { 0,10 });
		draw_line({ 10,0 }, { 20,10 });
		draw_line({ 5,5 }, {15,5});

		draw_line({ 30, 0 }, { 30,10 });
		draw_line({ 30, 10 }, { 40,10 });
		draw_line({ 50, 0 }, { 50,10 });
		draw_line({ 50, 10 }, { 60,10 });

		pencil(rainbow(1000));

		draw_circle({75,5},5);

		draw_clear();
	}

	return 0;
}