
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
* ou si tu est plus ambitieux, tu peut essayer de le faire toi m�me
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
	string seq1 = "  <line1;lin\"f;ds\"e2;line3<subline1; subline2>end;fdsfdfs>";
	parser.parse_sequence(seq1);

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

		if (key_released(SDL_SCANCODE_L)) //quand 'L' est relach�
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

		//Nouvelle fonctionnalit� !
		//Entrer la commande 'list all' :)
		track_variable(pos, "pos");
		track_variable(test_variable, "test");

		if (key_released(SDL_SCANCODE_1)) 
		{
			p2p().host();
			p2p().wait_for_peer();
		}

		if (key_released(SDL_SCANCODE_2))
		{
			p2p().join("192.168.138.31");
		}
		
		if (p2p().is_connected())
		{
			if (key_pressed(SDL_SCANCODE_X))
			{
				int i1 = r.range(0, 100);
				int i2 = r.range(0, 100);
				int i3 = r.range(0, 100);
				//p2p().start_stream(0); //on commence un canal '0', puis on envoie les donn�es
				//p2p().send(i1);
				//p2p().send(i2);
				//p2p().send(i3);
				//p2p().end_stream();
				p2p(0) << i1 << i2 << i3 << net::send;
				std::cout << i1 << " " << i2 << " " << i3 << " " << std::endl;
			}

			if (key_pressed(SDL_SCANCODE_Z))
			{
				int i1, i2, i3;
				//net.wait_for_stream(0); //On attend que le canal '0' arrive, puis on lit les donn�es
				//net.read_stream(0, i1, i2, i3);
				p2p()[0] >> net::wait >> i1 >> i2 >> i3;
				std::cout << i1 << " " << i2 << " " << i3 << " " << std::endl;
			}

			if (key_pressed(SDL_SCANCODE_E))
			{
				string s;
				std::cin >> s;
				//p2p().start_stream(1); //BTW on peut pas envoyer directement des conteneurs (vector, string, etc) parce qu'ils ne contiennent pas vraiment les donn�es, mais plutot des pointers VERS les donn�es (qui ne seront pas valide sur lordinateur de lautre)
				//p2p().send(s.size()); //on envoie la taille de la string
				//for (auto& c : s)
				//	p2p().send(c); //puis on envoie chaque carat�re un a la fois
				//p2p().end_stream();
				p2p()[1] << s.size();
				for (auto& c : s)
					p2p()[1] << c;
				p2p()[1] << net::send;
			}

			if (p2p().has_stream(1)) //Une bonne id�e de pratique avant de faire un jeu: faire un 'chat' ou les utilisateurs peuvent envoyer des messages entre eux
			{	
				string s;
				//size_t size = p2p().read<size_t>(1);
				size_t size;
				p2p()[1] >> size;
				for (size_t i = 0; i < size; i++)
				{
					//s.push_back(p2p().read<char>(1));
					char c;
					p2p()[1] >> c;
					s.push_back(c);

				}
				std::cout << s << std::endl;

				//pour entrer du texte sans std::cin
				keyboard().openTextInput(); // commence la collection de texte entr� sur le clavier
				keyboard().getTextInput(); // r�cupere le texte entr�
				keyboard().closeTextInput();
			}
		}
	}

	return 0;
}