#include "ludo.h"


vector<tile> carreaux;
vector<tile> get_tiles()
{
	vector<tile> tiles;
	for (int i = 0; i < 15 * 15; i++)
	{
		tiles.push_back(tile());
	}
	int index = 0;
	for (int i = BEG_Y_MAP; i <= END_Y_MAP - yx; i += yx)
	{
		for (int n = BEG_X_MAP; n <= END_X_MAP - xy; n += xy)
		{
			tiles.at(index).pos = { n,i };
			tiles.at(index).numero = index;
			index++;
		}
	}
	return tiles;
}


vector<int> chemin;
vector<int> get_chemin()
{
	vector<int> chemin;

	chemin = { 6,7,8,23,38,53,68, 83,99,100,101,102,103,104,119,134,133,132,131,130,129,143,158,173,188,203,218,217,216,
			   201,186,171,156,141,125,124,123,122,121,120,105,90,91,92,93,94,95,81,66,51,36,21};

	return chemin;
}

void init_game()
{
	carreaux = get_tiles();
	chemin = get_chemin();
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

	pencil((*vert).couleur);
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

void draw_board()
{
	for (int i = 0; i < carreaux.size(); i++)
	{
		if (trouver_colonne(i) < 6 && trouver_rangee(i) < 6)
		{
			pencil(rouge->couleur);
			carreaux.at(i).light();
		}
		else if (trouver_colonne(i) < 6 && trouver_rangee(i) >= 9)
		{
			pencil(bleu->couleur);
			carreaux.at(i).light();
		}
		else if (trouver_colonne(i) >= 9 && trouver_rangee(i) < 6)
		{
			pencil(jaune->couleur);
			carreaux.at(i).light();
		}
		else if (trouver_colonne(i) >= 9 && trouver_rangee(i) >= 9)
		{
			pencil(vert->couleur);
			carreaux.at(i).light();
		}
		else
		{
			pencil(COLOR_WHITE);
			carreaux.at(i).light();
		}
	}
	light_random_cases();
	draw_triangles();
}

pion* test = new pion(1,7);
int main()
{
	set_window_size(window);
	set_window_resizable();
	setlocale(LC_ALL, "");
	init_game();
	int ind = 0;
	test->pos = carreaux.at(chemin.at(ind)).pos + xy / 2;
	while (run())
	{
		pencil(COLOR_BLACK);
		draw_clear();
		draw_board();
		for (int i = 0; i < carreaux.size(); i++)
		{
			pencil(COLOR_BLACK);
			carreaux.at(i).show_num();
		}
		pencil(rgb(0,255,255));
		draw_full_circle(test->pos, test->rayon);
		if (mouse_left_pressed())
		{
			ind++;
			if (ind == chemin.size())
			{
				ind = 0;
			}
		}
		test->pos = carreaux.at(chemin.at(ind)).pos + xy / 2;
		rouge->display_tokens(carreaux);
		bleu->display_tokens(carreaux);
		vert->display_tokens(carreaux);
		jaune->display_tokens(carreaux);
		int num = de::shuffle();
		std::cout << num;
	}
}