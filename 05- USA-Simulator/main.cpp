#include "Shooting.h"

int main()
{
	set_window_size(window);
	set_window_resizable();

	player joueur;

	while (run())
	{
		pencil(COLOR_BLACK);
		draw_clear();
		joueur.show();
		joueur.move();
	}
}