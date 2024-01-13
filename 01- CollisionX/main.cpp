// Débuté le 10 janvier 2024


// Note à moi-même: À chaque fois que je crée un projet, je dois ajouter tous les fichiers .dll ainsi que le dossier Font qui
// sont tous dans le dossier Bruhuh


#include "balls.h"

int main()
{
	setlocale(LC_ALL, "");

	V2d_i window = {X_CONSOLE,Y_CONSOLE};
	set_window_size(window);
	set_window_resizable();

	vector<square> vect;
	for (int i = 0; i < 5; i++)
	{
		vect.push_back(square());
	}
	while (run())
	{
		pencil(COLOR_BLACK);
		draw_clear();
		pencil(COLOR_GREEN);
		for (int i = 0; i < vect.size(); i++)
		{
			vect.at(i).create();
		}
		if (mouse_left_pressed())
		{
			vect.push_back(square());
		}
		for (int i = 0; i < vect.size(); i++)
		{
			vect.at(i).boink(vect);
		}
	}
}