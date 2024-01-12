// Débuté le 10 janvier 2024


// Note à moi-même: À chaque fois que je crée un projet, je dois ajouter tous les fichiers .dll ainsi que le dossier Font qui
// sont tous dans le dossier Bruhuh


#include "balls.h"

square nouveau()
{
	square carre;
	return carre;
}

int main()
{
	setlocale(LC_ALL, "");

	circle cercle;
	square carre;
	square a;
	square b;
	square c;
	square d;
	square e;
	V2d_i window = {X_CONSOLE,Y_CONSOLE};
	set_window_size(window);
	set_window_resizable();

	vector<V2d_i> rectangles;
	vector<square> vect = { a,b,c,d };
	while (run())
	{
		pencil(COLOR_GREEN);
		for (int i = 0; i < vect.size(); i++)
		{
			vect.at(i).create();
		}
		if (mouse_left_pressed())
		{
			vect.push_back(nouveau());
		}
		for (int i = 0; i < vect.size(); i++)
		{
			/*if (vect.at(i).boink(vect))
			{
				vect.push_back(nouveau());
			}*/
		}
	}
}