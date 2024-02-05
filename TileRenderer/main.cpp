
#include "TileRenderer.h"

#include "CstarParser.h"
#include "File.h"
#include "Networking.h"

/*
* Ceci est un exemple!
* Tu peut regarder dans TileRenderer.h, il y a d'autres fonctions 
* (par exemple, tu peut dessiner des images... :) )
* 
* Ya pas encore de texte, j'imagine tu peut utiliser la console pour ca 
* ou si tu est plus ambitieux, tu peut essayer de le faire toi même
* 
* Juste a creer un nouveau proet dans la solution et copier les settings de 'exemple'
*/

int main() 
{
	set_window_size(200); //fenetre de 200x200
	set_window_resizable(); //fenetre peut etre agrandie

	/*
	* En gros ca marche plus ou moins de la meme facon que le console renderer
	* juste plus rapide et meilleur
	*/

	int test_variable = 0;

	CstarParser parser;
	File file("file.txt", FILE_READING_STRING);

	string str = file.getString();
	parser.parse(str);

	Peer2Peer net;
	Random r;

	while (run()) //boucle principale
	{
		pencil(COLOR_BLACK); //Permet d'enlever tout ce qui reste de la derniere frame
		draw_clear();

		pencil(rgb(255, 255, 0)); //Couleur jaune

		draw_line({ 10,0 }, { 0,10 }); //ligne

		draw_image("Images/chad.jpg", { {20,10},{50,25} }); // dessin de l'image (faut s'assurer que le nom est bon)

		pencil(rainbow(1000)); //arc-en-ciel, boucle de 1000ms

		draw_circle({ 75,5 }, 5); //cercle

		if (key_pressed(SDL_SCANCODE_SPACE))
			pencil(COLOR_GREEN);
		else
			pencil(COLOR_PINK);

		draw_full_rect({ 50,50 }); //rectangle plein
		draw_rect({ {50,102 },50 }); //rectangle pas plein

		if (mouse_left_held())
		{
			pencil(COLOR_WHITE);
			draw_circle(mouse_position(), 10); //cercle a la position de la souris
		}

		if (key_released(SDL_SCANCODE_L)) //quand 'L' est relaché
			std::cout << "James" << std::endl;

		if (mouse_right_pressed())
		{
			std::cout << random().range(1, 10) << std::endl;
			std::cout << random().frange(1, 10) << std::endl;
			sound().playSound("Sounds/rizz.wav");
		}

		int pos = (int)(sin((SDL_GetTicks() % 1000) / 1000.0) * 100) - 50;
		draw_simple_text("You have no bitches", { pos,60 }, get_font(1)); //get_font(1) voir 'Fonts/fonts.hint'
		draw_simple_text("You have " + strings::stringify(test_variable) + " bitches", { 0,120 }, get_font(0)); //get_font(0) voir 'Fonts/fonts.hint'

		//Nouvelle fonctionnalité !
		//Entrer la commande 'list all' :)
		track_variable(pos, "pos");
		track_variable(test_variable, "test");

		if (key_released(SDL_SCANCODE_1)) 
		{
			net.host();
			net.wait_for_peer();
		}

		if (key_released(SDL_SCANCODE_2))
		{
			net.join();
		}
		
		if (net.is_connected()) 
		{
			if (key_pressed(SDL_SCANCODE_X))
			{
				int i1 = r.range(0, 100);
				int i2 = r.range(0, 100);
				int i3 = r.range(0, 100);
				//net.start_stream(0); //on commence un canal '0', puis on envoie les données
				//net.send(i1);
				//net.send(i2);
				//net.send(i3);
				//net.end_stream();
				net[0] << i1 << i2 << i3 << net::send;
				std::cout << i1 << " " << i2 << " " << i3 << " " << std::endl;
			}

			if (key_pressed(SDL_SCANCODE_Z))
			{
				int i1, i2, i3;
				//net.wait_for_stream(0); //On attend que le canal '0' arrive, puis on lit les données
				//net.read_stream(0, i1, i2, i3);
				net[0] >> net::wait >> i1 >> i2 >> i3;
				std::cout << i1 << " " << i2 << " " << i3 << " " << std::endl;
			}

			if (key_pressed(SDL_SCANCODE_E))
			{
				string s;
				std::cin >> s;
				//net.start_stream(1); //BTW on peut pas envoyer directement des conteneurs (vector, string, etc) parce qu'ils ne contiennent pas vraiment les données, mais plutot des pointers VERS les données (qui ne seront pas valide sur lordinateur de lautre)
				//net.send(s.size()); //on envoie la taille de la string
				//for (auto& c : s)
				//	net.send(c); //puis on envoie chaque caratère un a la fois
				//net.end_stream();
				net[1] << s.size();
				for (auto& c : s)
					net[1] << c;
				net[1] << net::send;
			}

			if (net.has_stream(1)) //Une bonne idée de pratique avant de faire un jeu: faire un 'chat' ou les utilisateurs peuvent envoyer des messages entre eux
			{	
				string s;
				//size_t size = net.read<size_t>(1);
				size_t size;
				net[1] >> size;
				for (size_t i = 0; i < size; i++)
				{
					//s.push_back(net.read<char>(1));
					char c;
					net[1] >> c;
					s.push_back(c);

				}
				std::cout << s << std::endl;

				//pour entrer du texte sans std::cin
				keyboard().openTextInput(); // commence la collection de texte entré sur le clavier
				keyboard().getTextInput(); // récupere le texte entré
				keyboard().closeTextInput();
			}
		}
	}

	return 0;
}