// Débuté le 10 janvier 2024
// Terminé le 14/01/2024


// Note à moi-même: À chaque fois que je crée un projet, je dois ajouter tous les fichiers .dll ainsi que le dossier Font qui
// sont tous dans le dossier Bruhuh. Pour ce qui est des settings, je dois les copier dans mon nouveau projet et changer la 
// version de C++ (pour ce faire: Propriétés de configuration --> Généran -> Normes du languages --> C++17

#include "pch.h"

#include "collisionx_balls.h"
#include "collisionx_Strings.h"

namespace COLLISIONX
{

	void afficher_message_defaite(square& main, vector<square>& vect, bool& jouer, V2d_i& posFruit, bool& showFruit)
	{
		pencil(COLOR_WHITE);
		draw_simple_text("Vous avez perdu :(  Appuyez sur 'q' pour rejouer !", { X_CONSOLE / 2 - 200,Y_CONSOLE / 2 }, get_font(0));
		if (key_pressed(SDL_SCANCODE_Q))
		{
			//sound().stopMusic();
			pencil(COLOR_BLACK);
			draw_clear();
			main.main_touched = false;
			main.begin = false;
			jouer = true;
			for (int i = 0; i < 5; i++)
			{
				vect.push_back(square());
			}
			posFruit.x = random().range(0, X_CONSOLE - 20);
			posFruit.y = random().range(0, Y_CONSOLE - 20);
			showFruit = true;
		}
	}

	void show_fruit(V2d_i posFruit)
	{
		draw_image("alexandre.png", { posFruit,{30,30} });
	}

	void show_score(int score)
	{
		string scores;
		scores = entier_en_chaine(score);
		string phrase = "Votre score est: " + scores;
		pencil(COLOR_WHITE);
		draw_simple_text(phrase, { X_CONSOLE / 2,10 }, get_font(0));
	}

	void collisionx_main()
	{
		setlocale(LC_ALL, "");

		V2d_i window = { X_CONSOLE,Y_CONSOLE };
		set_window_size(window);
		set_window_resizable();

		vector<square> vect;
		for (int i = 0; i < 5; i++)
		{
			vect.push_back(square());
		}

		// main character
		square main;
		main.is_main = true;
		main.velocite = 0;

		// Fruit
		V2d_i posFruit;
		posFruit.x = random().range(0, X_CONSOLE - 20);
		posFruit.y = random().range(0, Y_CONSOLE - 20);
		bool showFruit = true;

		// Score
		int score = 0;
		const int scoreIncrease = 100;

		int pause = 1;

		while (run())
		{
			if (key_pressed(SDL_SCANCODE_SPACE))
			{
				pause *= -1;
			}
			if (pause == 1)
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
				pencil(COLOR_WHITE);
				if (!main.main_touched)
				{
					main.create();
					main.show_coordinates();
					main.boink(vect);
				}
				else
				{
					showFruit = false;
					bool jouer = false;
					if (!jouer)
					{
						afficher_message_defaite(main, vect, jouer, posFruit, showFruit);
						score = 0;
					}
				}
				if (showFruit)
				{
					show_fruit(posFruit);
					show_score(score);
					if (main.get_pos().x < posFruit.x + 30 &&
						main.get_pos().x + 30 > posFruit.x &&
						main.get_pos().y < posFruit.y + 30 &&
						main.get_pos().y + 30 > posFruit.y)
					{
						posFruit.x = random().range(0, X_CONSOLE - 20);
						posFruit.y = random().range(0, Y_CONSOLE - 20);
						sound().stopMusic();
						sound().playSound("eatfruit.wav");
						vect.push_back(square());
						score += scoreIncrease;
					}
				}
			}
		}
	}
}

#include "GLUU.h"

namespace COLLISIONX {
	GLUU_IMPORT_MAIN(collisionx_main);
}