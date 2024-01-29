#include "Shooting.h"

int main()
{ 
	// Site pour les sound effects: myinstants.com

	set_window_size(window);
	set_window_resizable();

	player joueur;
	vector<bullets> balles;
	vector<balls> ballons;
	int ballNum = 5;
	for (int i = 0; i < ballNum; i++)
	{
		ballons.push_back(balls());
	}

	while (run())
	{
		pencil(COLOR_BLACK);
		draw_clear();
		joueur.show();
		joueur.move();
		if (mouse_left_pressed())
		{
			sound().playSound("piou.wav");
;			balles.push_back(bullets());
		}
		for (int i = 0; i < balles.size(); i++)
		{
			if (!balles.at(i).is_on_screen())
			{
				balles.erase(balles.begin() + i);
				continue;
			}
			balles.at(i).shoot({ joueur.get_pos().x + (joueur.get_xy().x / 2), joueur.get_pos().y - balles.at(i).get_xy().y});
		}
		for (int i = 0; i < ballons.size(); i++)
		{
			ballons.at(i).show();
			ballons.at(i).move();
			ballons.at(i).show_number();
			ballons.at(i).boom(i,balles,ballons);
		}
	}
}