#include "ludo.h"

void draw_triangle(V2d_i pos1, V2d_i pos2, V2d_i pos3)
{
	draw_line(pos1, pos2);
	draw_line(pos2, pos3);
	draw_line(pos3, pos1);
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

void draw_board_triangles()
{
	pencil(bleu.couleur);
	draw_triangle(bleu1,centre, bleu2);

	pencil(rouge.couleur);
	draw_triangle(rouge1, centre,rouge2);

	pencil(vert.couleur);
	draw_triangle(vert1, centre, vert2);

	pencil(jaune.couleur);
	draw_triangle(jaune1, centre, jaune2);
}


void draw_board()
{
	pencil(COLOR_CYAN);
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

	draw_board_triangles();
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