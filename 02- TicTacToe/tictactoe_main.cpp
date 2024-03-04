#include "tictactoe_game.h"

// Débuté le 14/01/2024
// Terminé le même jour après presque 4h de codage (c'était assez facile)

void show_name()
{
	V2d_i pos = { BEG_X_MAP + (END_X_MAP - BEG_X_MAP) * 1 / 4 + 50, BEG_Y_MAP - 50 };
	pencil(COLOR_GREEN);
	draw_simple_text("Tic-Tac-Toe", pos, get_font(0));
}

void draw_lines()
{
	pencil(COLOR_WHITE);
	int squaresPerRow = 3;
	draw_rect({ {BEG_X_MAP,BEG_Y_MAP},{END_X_MAP - BEG_X_MAP,END_Y_MAP - BEG_Y_MAP} });
	int divisions = (END_X_MAP - BEG_X_MAP) / squaresPerRow;
	for (int i = 1; i <= squaresPerRow - 1; i++)
	{
		draw_line({ BEG_X_MAP + divisions * i, BEG_Y_MAP }, { BEG_X_MAP + divisions * i, END_Y_MAP });
		draw_line({ BEG_X_MAP, BEG_Y_MAP + divisions * i }, { END_X_MAP, BEG_Y_MAP + divisions * i });
	}
}

void switch_turns(player& joueur1, player& joueur2)
{
	if (joueur1.has_turn)
	{
		joueur1.has_turn = false;
		joueur2.has_turn = true;
	}
	else
	{
		joueur1.has_turn = true;
		joueur2.has_turn = false;
	}
}

vector<V2d_i> get_squares()
{
	vector<V2d_i> squares;
	int squaresPerRow = 3;
	int squaresPerColumn = 3;
	int xy = (END_X_MAP - BEG_X_MAP) / squaresPerColumn;
	int yx = (END_Y_MAP - BEG_Y_MAP) / squaresPerRow;

	for (int i = BEG_Y_MAP; i <= END_Y_MAP - yx; i += yx)
	{
		for (int n = BEG_X_MAP; n <= END_X_MAP - xy; n += xy)
		{
			squares.push_back({ n,i });
		}
	}
	return squares;
}

player bot;
bool botPlay = true;
player joueur1;
player joueur2;

player& actualPlayer()
{
	if (joueur1.has_turn)
	{
		return joueur1;
	}
	return joueur2;
}

player& notActualPlayer()
{
	if (joueur1.has_turn)
	{
		return joueur2;
	}
	return joueur1;
}

int click_check(vector<V2d_i> squares, vector<int>& carresCheckes, player& joueur1, player& joueur2)
{
	int squaresPerRow = 3;
	int squaresPerColumn = 3;
	int xy = (END_X_MAP - BEG_X_MAP) / squaresPerColumn;
	int yx = (END_Y_MAP - BEG_Y_MAP) / squaresPerRow;

	for (int i = 0; i < squares.size(); i++)
	{
		if (mouse().position().x > squares.at(i).x && mouse().position().x < squares.at(i).x + xy &&
			mouse().position().y > squares.at(i).y && mouse().position().y < squares.at(i).y + yx)
		{
			for (int n = 0; n < carresCheckes.size(); n++)
			{
				if (carresCheckes.at(n) == i + 1)
				{
					return -1;
				}
			}
			switch_turns(joueur1, joueur2);
			draw_full_rect({ squares.at(i),{xy,yx} });
			carresCheckes.push_back(i + 1);
			return i + 1;
		}
	}
	return -1;
}

bool is_in(int element, vector<int> lesElements)
{
	for (int i = 0; i < lesElements.size(); i++)
	{
		if (lesElements.at(i) == element)
		{
			return true;
		}
	}
	return false;
}

bool win_check(player& joueur)
{
	if (is_in(1,joueur.squaresPossessed) && is_in(2, joueur.squaresPossessed) && is_in(3, joueur.squaresPossessed))
	{
		return true;
	}
	else if (is_in(1, joueur.squaresPossessed) && is_in(4, joueur.squaresPossessed) && is_in(7, joueur.squaresPossessed))
	{
		return true;
	}
	else if (is_in(1, joueur.squaresPossessed) && is_in(5, joueur.squaresPossessed) && is_in(9, joueur.squaresPossessed))
	{
		return true;
	}
	else if (is_in(7, joueur.squaresPossessed) && is_in(8, joueur.squaresPossessed) && is_in(9, joueur.squaresPossessed))
	{
		return true;
	}
	else if (is_in(4, joueur.squaresPossessed) && is_in(5, joueur.squaresPossessed) && is_in(6, joueur.squaresPossessed))
	{
		return true;
	}
	else if (is_in(3, joueur.squaresPossessed) && is_in(5, joueur.squaresPossessed) && is_in(7, joueur.squaresPossessed))
	{
		return true;
	}
	else if (is_in(3, joueur.squaresPossessed) && is_in(6, joueur.squaresPossessed) && is_in(9, joueur.squaresPossessed))
	{
		return true;
	}
	return false;

}

void anounce_winner(player gagnant)
{
	pencil(COLOR_WHITE);
	string phrase = "Le " + gagnant.name + " est victorieux!";
	draw_simple_text(phrase, { BEG_X_MAP,END_Y_MAP + 20 }, get_font(0));
}

void announce_draw()
{
	pencil(COLOR_WHITE);
	string phrase = "Personne ne gagne!";
	draw_simple_text(phrase, { BEG_X_MAP,END_Y_MAP + 20 }, get_font(0));
}

bool recommencer(V2d_i posBouton, V2d_i xy)
{
	if (mouse().position().x > posBouton.x && mouse().position().x < posBouton.x + xy.x &&
		mouse().position().y > posBouton.y && mouse().position().y < posBouton.y + xy.y)
	{
		return true;
	}
	return false;
}

void reinitialiser_joueurs(player& joueur1, player& joueur2)
{
	joueur1.has_turn = true;
	joueur2.has_turn = false;
	joueur1.squaresPossessed.clear();
	joueur2.squaresPossessed.clear();
}

void tictactoe_main()
{
	setlocale(LC_ALL, "");

	vector<V2d_i> squares = get_squares();
	
	joueur1.has_turn = true;
	joueur2.has_turn = false;

	joueur1.name = "Joueur1";
	joueur2.name = "Joueur2";

	bool modifiable = true;

	V2d_i window = { X_CONSOLE, Y_CONSOLE };
	set_window_size(window);
	set_window_resizable();

	int numeroCheck;
	vector<int> carresCheckes;

	V2d_i boutonRecommencer = {END_X_MAP - 50,END_Y_MAP + 20};
	V2d_i xyRecommencer = {50,30};
	
	while (run())
	{
		show_name();
		draw_lines();
		draw_full_rect(Rect({ boutonRecommencer,xyRecommencer }));
		pencil(COLOR_PINK);
		draw_simple_text("Restart", { END_X_MAP + 10,END_Y_MAP + 30 }, get_font(0));
		if (joueur1.has_turn)
		{
			pencil(COLOR_GREEN);
		}
		else
		{
			pencil(COLOR_CYAN);
		}
		if (mouse_left_pressed() && modifiable)
		{
			numeroCheck = click_check(squares, carresCheckes, joueur1, joueur2);
			actualPlayer().squaresPossessed.push_back(numeroCheck);
		}
		if (carresCheckes.size() >= 5)
		{
			if (win_check(actualPlayer()))
			{
				modifiable = false;
				anounce_winner(notActualPlayer());
			}
			else if (carresCheckes.size() == 9)
			{
				modifiable = false;
				announce_draw();
			}
		}
		if (mouse_left_pressed())
		{
			if (recommencer(boutonRecommencer,xyRecommencer))
			{
				modifiable = true;;
				pencil(COLOR_BLACK);
				draw_full_rect(Rect({ {0,0},{X_CONSOLE,Y_CONSOLE} }));
				draw_lines();
				reinitialiser_joueurs(joueur1, joueur2);
				carresCheckes.clear();
			}
		}
	}
}

#include "GLUU.h"

GLUU_IMPORT_MAIN(tictactoe_main);
