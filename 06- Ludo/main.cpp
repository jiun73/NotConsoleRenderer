#include "ludo.h"

environment jeu;

void init_game()
{
	carreaux = get_tiles();
}

void draw_lines()
{
	pencil(COLOR_BLACK);
	draw_rect({ {BEG_X_MAP,BEG_Y_MAP},{END_X_MAP - BEG_X_MAP,END_Y_MAP - BEG_Y_MAP} });
	for (int i = 1; i <= squaresPerColumn - 1; i++)
	{
		draw_line({ BEG_X_MAP + xy * i, BEG_Y_MAP }, { BEG_X_MAP + xy * i, END_Y_MAP });
	}
	for (int i = 1; i <= squaresPerRow - 1; i++)
	{
		draw_line({ BEG_X_MAP, BEG_Y_MAP + yx * i }, { END_X_MAP, BEG_Y_MAP + yx * i });
	}
}

void init_players(player& rouge, player& bleu, player& jaune, player& vert)
{
	rouge.couleur = rgb(255, 0, 0);
	bleu.couleur = rgb(0, 0, 255);
	jaune.couleur = rgb(255, 255, 0);
	vert.couleur = rgb(0, 255, 0);
	rouge.name = "rouge";
	bleu.name = "bleu";
	jaune.name = "jaune";
	vert.name = "vert";
}

void light_random_cases()
{
	pencil(rouge->couleur);
	carreaux.at(91).light();
	carreaux.at(106).light();
	carreaux.at(107).light();
	carreaux.at(108).light();
	carreaux.at(109).light();
	carreaux.at(110).light();

	pencil(jaune->couleur);
	carreaux.at(22).light();
	carreaux.at(23).light();
	carreaux.at(37).light();
	carreaux.at(52).light();
	carreaux.at(67).light();
	carreaux.at(82).light();

	pencil(bleu->couleur);
	carreaux.at(201).light();
	carreaux.at(202).light();
	carreaux.at(187).light();
	carreaux.at(172).light();
	carreaux.at(157).light();
	carreaux.at(142).light();

	pencil(vert->couleur);
	carreaux.at(133).light();
	carreaux.at(118).light();
	carreaux.at(117).light();
	carreaux.at(116).light();
	carreaux.at(115).light();
	carreaux.at(114).light();
}

int trouver_rangee(int index)
{
	return index / 15;
}

int trouver_colonne(int index)
{
	return index % 15;
}

void draw_triangles()
{
	V2d_i centre = { carreaux.at(112).pos.x + xy / 2, carreaux.at(112).pos.y + yx / 2 };
	V2d_i topleft = { carreaux.at(96).pos.x, carreaux.at(96).pos.y };
	V2d_i bottomleft = { carreaux.at(141).pos.x, carreaux.at(141).pos.y };
	V2d_i topright = { carreaux.at(99).pos.x, carreaux.at(99).pos.y };
	V2d_i bottomright = { carreaux.at(144).pos.x, carreaux.at(144).pos.y };
	pencil(rouge->couleur);
	draw_full_triangle(bottomleft, topleft, centre);
	pencil(jaune->couleur);
	draw_full_triangle(topright, topleft, centre);
	pencil(vert->couleur);
	draw_full_triangle(bottomright, topright, centre);
	pencil(bleu->couleur);
	draw_full_triangle(bottomright, centre, bottomleft);
	pencil(COLOR_BLACK);
	draw_rect({ topleft, bottomright - topleft });
	draw_line(topleft,bottomright);
	draw_line(bottomleft, topright);
}

void draw_white_spots(int num)
{
	pencil(COLOR_WHITE);
	draw_full_rect({ {carreaux.at(num).pos},{xy * 4, yx * 4}});
}

void draw_board()
{
	int ex = 0;
	for (int i = 0; i < carreaux.size(); i++)
	{
		if (trouver_colonne(i) < 6 && trouver_rangee(i) < 6)
		{
			pencil(rouge->couleur);
			carreaux.at(i).light();
			draw_white_spots(16);
		}
		else if (trouver_colonne(i) < 6 && trouver_rangee(i) >= 9)
		{
			
			pencil(bleu->couleur);
			carreaux.at(i).light();
			draw_white_spots(151);
		}
		else if (trouver_colonne(i) >= 9 && trouver_rangee(i) < 6)
		{
			
			pencil(jaune->couleur);
			carreaux.at(i).light();
			draw_white_spots(25);
		}
		else if (trouver_colonne(i) >= 9 && trouver_rangee(i) >= 9)
		{
			
			pencil(vert->couleur);
			carreaux.at(i).light();
			draw_white_spots(160);
		}
		else
		{
			pencil(COLOR_WHITE);
			carreaux.at(i).light();
		}
	}
	light_random_cases();
	draw_triangles();
	for (int i = 0; i < carreaux.size(); i++)
	{
		pencil(COLOR_BLACK);
		carreaux.at(i).show_num();
	}
}

void display_tokens()
{
	rouge->display_tokens();
	bleu->display_tokens();
	vert->display_tokens();
	jaune->display_tokens();
}

void switch_turns()
{
	if (!jeu.pionMovement)
	{
		if (rouge->is_playing)
		{
			rouge->is_playing = false;
			bleu->is_playing = true;
		}
		else if (bleu->is_playing)
		{
			bleu->is_playing = false;
			vert->is_playing = true;
		}
		else if (vert->is_playing)
		{
			vert->is_playing = false;
			jaune->is_playing = true;
		}
		else if (jaune->is_playing)
		{
			jaune->is_playing = false;
			rouge->is_playing = true;
		}
	}
}

bool pion_sorti(player joueur)
{
	if (joueur.token1->outOfHome)
	{
		return true;
	}
	if (joueur.token2->outOfHome)
	{
		return true;
	}
	if (joueur.token3->outOfHome)
	{
		return true;
	}
	if (joueur.token4->outOfHome)
	{
		return true;
	}
	return false;
}

void click_on_token(player& joueur, bool& valid)
{
	const tile& token1 = carreaux.at(joueur.token1->caseActuelle);
	const tile& token2 = carreaux.at(joueur.token2->caseActuelle);
	const tile& token3 = carreaux.at(joueur.token3->caseActuelle);
	const tile& token4 = carreaux.at(joueur.token4->caseActuelle);


	
	if (point_in_rectangle(mouse_position(), { token1.pos, token1.pos + xy / 2 }))
	{
		valid = true;
		joueur.token1->caseActuelle = joueur.spawnTile;
	}
	else if (point_in_rectangle(mouse_position(), { token2.pos, token2.pos + xy / 2 }))
	{
		valid = true;
		joueur.token2->caseActuelle = joueur.spawnTile;
	}
	else if (point_in_rectangle(mouse_position(), { token3.pos, token3.pos + xy / 2 }))
	{
		valid = true;
		joueur.token3->caseActuelle = joueur.spawnTile;
	}
	else if (point_in_rectangle(mouse_position(), { token4.pos, token4.pos + xy / 2 }))
	{
		valid = true;
		joueur.token4->caseActuelle = joueur.spawnTile;
	}
}

int jouer_son_tour(player& joueur, int des)
{
	if (!pion_sorti(joueur))
	{
		if (des == 6)
		{
			draw_simple_text("Clickez sur un de vos pions ...", {10,10}, get_font(0));
			/*while (true)
			{
				bool valid = false;
				if (mouse_left_pressed())
				{
					click_on_token(joueur, valid);
					if (valid)
					{
						jeu.pionMouvement = false;
						return 1;
					}
				}
			}*/
		}
		return -1;
	}
	return -1;
}


int main()
{
	set_window_size(window);
	set_window_resizable();
	setlocale(LC_ALL, "");
	init_game();
	int compteur = 0;
	bool fait = true;
	int des;
	rouge->is_playing = true;


	while (run())
	{
		pencil(COLOR_BLACK);
		draw_clear();
		draw_board();

		draw_simple_text(actual().name, {700,10}, get_font(0));
		des = de::shuffle();
		compteur++;
		
		jouer_son_tour(actual(), des);
		
		pencil(rgb(0, 255, 255));
		switch_turns();
	}
}