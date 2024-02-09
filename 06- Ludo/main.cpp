#include "ludo.h"

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
	vert.couleur = rgb(0, 0, 255);
	rouge.name = "rouge";
	bleu.name = "bleu";
	jaune.name = "jaune";
	vert.name = "vert";
}


void draw_board()
{
	pencil(COLOR_GREEN);
	draw_full_rect(board_contour);
	draw_lines();

	pencil(bleu.couleur);
	draw_full_rect(le_bleu);

	pencil(rouge.couleur);
	draw_full_rect(le_rouge);

	pencil(jaune.couleur);
	draw_full_rect(le_jaune);

	pencil(vert.couleur);
	draw_full_rect(le_vert);
}


int main()
{
	set_window_size(window);
	set_window_resizable();
	setlocale(LC_ALL, "");
	init_players(rouge, bleu, jaune, vert);

	while (run())
	{
		pencil(COLOR_WHITE);
		draw_clear();

		draw_board();
	}
}