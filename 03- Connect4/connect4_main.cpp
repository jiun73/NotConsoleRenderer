#include "connect4_game.h"
#include "Strings.h"

// Débuté le 15/01/2024
// Terminé le même jour après presque 2h de codage (c'était assez facile, étant donné que je me suis inspiré de tic-tac-toe pour le faire)

namespace CONNECT4
{

	void show_name()
	{
		string num = entier_en_chaine(connexionsAFaire);
		string phrase = "Connect " + num;
		V2d_i pos = { BEG_X_MAP + (END_X_MAP - BEG_X_MAP) * 1 / 4 + 50, BEG_Y_MAP - 50 };
		pencil(COLOR_GREEN);
		draw_simple_text(phrase, pos, get_font(0));
	}

	void draw_lines()
	{
		pencil(COLOR_WHITE);
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

	void update_modifiable(vector<V2d_i> squares, vector<bool> bouleen)
	{
		pencil(COLOR_WHITE);
		for (int i = 0; i < squares.size(); i++)
		{
			if (!bouleen.at(i))
			{
				draw_line({ squares.at(i) }, { squares.at(i).x + xy, squares.at(i).y + yx });
			}
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


		for (int i = BEG_Y_MAP; i <= END_Y_MAP - yx; i += yx)
		{
			for (int n = BEG_X_MAP; n <= END_X_MAP - xy; n += xy)
			{
				squares.push_back({ n,i });
			}
		}
		return squares;
	}

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

	void remove_line(V2d_i pos)
	{
		pencil(COLOR_BLACK);
		draw_full_rect(Rect({ pos,{xy,yx} }));
	}

	int click_check(vector<V2d_i> squares, vector<bool>& squaresModifiable, vector<int>& carresCheckes, player& joueur1, player& joueur2)
	{
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
				if (squaresModifiable.at(i))
				{
					switch_turns(joueur1, joueur2);
					draw_full_rect({ squares.at(i),{xy,yx} });
					if (i >= squaresPerColumn)
					{
						remove_line(squares.at(i - squaresPerColumn));
						squaresModifiable.at(i - squaresPerColumn) = true;
					}
					carresCheckes.push_back(i + 1);
					return i + 1;
				}
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

	bool up_connect(int numero)
	{
		if (numero <= squaresPerColumn * (connexionsAFaire - 1))
		{
			return false;
		}
		int add;
		for (int i = 0; i < connexionsAFaire - 1; i++)
		{
			add = numero - (i + 1) * up_difference;
			if (!is_in(add, actualPlayer().squaresPossessed))
			{
				return false;
			}
		}
		return true;
	}

	bool down_connect(int numero)
	{
		if (numero > squaresPerColumn * (connexionsAFaire - 1))
		{
			return false;
		}
		int add = numero;
		for (int i = 0; i < connexionsAFaire - 1; i++)
		{
			add += up_difference;
			if (!is_in(add, actualPlayer().squaresPossessed))
			{
				return false;
			}
		}
		return true;
	}

	bool updiag_connect(int numero)
	{
		int add = numero;
		for (int i = 0; i < connexionsAFaire - 1; i++)
		{
			add -= up_diag_difference;
			if (!is_in(add, actualPlayer().squaresPossessed))
			{
				return false;
			}
		}
		return true;
	}

	bool downdiag_connect(int numero)
	{
		int add = numero;
		for (int i = 0; i < connexionsAFaire - 1; i++)
		{
			add += down_diag_difference;
			if (!is_in(add, actualPlayer().squaresPossessed))
			{
				return false;
			}
		}
		return true;
	}

	int get_row(int numero)
	{
		int row = numero % squaresPerColumn;
		if (numero % squaresPerColumn != 0)
		{
			row++;
		}
		return row;
	}

	bool left_connect(int numero)
	{
		int add = numero;
		for (int i = 0; i < connexionsAFaire - 1; i++)
		{
			add -= side_difference;
			if (!is_in(add, actualPlayer().squaresPossessed))
			{
				return false;
			}
		}
		return true;
	}

	bool right_connect(int numero)
	{
		int add = numero;
		for (int i = 0; i < connexionsAFaire - 1; i++)
		{
			add += side_difference;
			if (!is_in(add, actualPlayer().squaresPossessed))
			{
				return false;
			}
		}
		return true;
	}

	bool win_check(player& joueur)
	{
		for (int i = 0; i < joueur.squaresPossessed.size(); i++)
		{
			if (up_connect(joueur.squaresPossessed.at(i)) ||
				down_connect(joueur.squaresPossessed.at(i)) ||
				updiag_connect(joueur.squaresPossessed.at(i)) ||
				downdiag_connect(joueur.squaresPossessed.at(i)) ||
				left_connect(joueur.squaresPossessed.at(i)) ||
				right_connect(joueur.squaresPossessed.at(i)))
			{
				return true;
			}
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

	vector<bool> squaresOccupied(vector<V2d_i> vecteur)
	{
		vector<bool> v;
		for (int i = 0; i < vecteur.size() - squaresPerColumn; i++)
		{
			v.push_back(false);
		}
		for (int i = 0; i < squaresPerColumn; i++)
		{
			v.push_back(true);
		}
		return v;
	}

	void connect4_main()
	{
		setlocale(LC_ALL, "");

		bool firstUse = true;

		vector<V2d_i> squares = get_squares();
		vector<bool> squaresModifiable = squaresOccupied(squares);

		joueur1.has_turn = true;
		joueur2.has_turn = false;

		joueur1.name = "jaune";
		joueur2.name = "rouge";

		bool modifiable = true;

		V2d_i window = { X_CONSOLE, Y_CONSOLE };
		set_window_size(window);
		set_window_resizable();

		int numeroCheck;
		vector<int> carresCheckes;

		V2d_i boutonRecommencer = { END_X_MAP - 50,END_Y_MAP + 20 };
		V2d_i xyRecommencer = { 50,30 };

		while (run())
		{
			sound().playSound("music.mp3", 6468);
			Mix_VolumeMusic(128);
			sound().resumeMusic();

			if (firstUse)
			{
				update_modifiable(squares, squaresModifiable);
				firstUse = false;
			}
			show_name();
			draw_lines();
			draw_full_rect(Rect({ boutonRecommencer,xyRecommencer }));
			pencil(COLOR_PINK);
			draw_simple_text("Restart", { END_X_MAP + 10,END_Y_MAP + 30 }, get_font(0));
			if (joueur1.has_turn)
			{
				pencil(rgb(255, 255, 0));
			}
			else
			{
				pencil(rgb(255, 0, 0));
			}
			if (mouse_left_pressed() && modifiable)
			{
				if (carresCheckes.size() == 15)
				{
					std::cout << "skvn";
				}
				numeroCheck = click_check(squares, squaresModifiable, carresCheckes, joueur1, joueur2);
				if (numeroCheck > 0)
				{
					actualPlayer().squaresPossessed.push_back(numeroCheck);
				}
			}
			if (carresCheckes.size() >= connexionsAFaire * 2 - 1)
			{
				if (win_check(actualPlayer()))
				{
					modifiable = false;
					anounce_winner(notActualPlayer());
				}
				else if (carresCheckes.size() == squares.size())
				{
					modifiable = false;
					announce_draw();
				}
			}
			if (mouse_left_pressed())
			{
				if (recommencer(boutonRecommencer, xyRecommencer))
				{
					modifiable = true;;
					pencil(COLOR_BLACK);
					draw_full_rect(Rect({ {0,0},{X_CONSOLE,Y_CONSOLE} }));
					draw_lines();
					reinitialiser_joueurs(joueur1, joueur2);
					carresCheckes.clear();
					firstUse = true;
					squaresModifiable = squaresOccupied(squares);
				}
			}
		}
	}


	
}

#include "pch.h"

namespace CONNECT4 
{
	GLUU_IMPORT_MAIN(connect4_main);
}
