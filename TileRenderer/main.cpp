
#include "NotConsoleRenderer.h"

#include "GLUU.h"
#include "File.h"
#include "Networking.h"
#include "CommandStandard.h"

/*
* Ceci est un exemple!
* Tu peut regarder dans TileRenderer.h, il y a d'autres fonctions 
* (par exemple, tu peut dessiner des images... :) )
* 
* Ya pas encore de texte, j'imagine tu peut utiliser la console pour ca 
* ou si tu est plus ambitieux, tu peut essayer de le faire toi même
* 
* Juste a creer un nouveau proet dans la solution et copier les settings de 'exemple'
* 
* TODO:	GLUU styler
*		GLUU better error handling
*		GLUU++
*		Chatroom
*		AnimationMaker
*/

enum MessageTypes 
{
	MESSAGE_IN,
	MESSAGE_OUT,
	SET_NAME, //string
	NAME_UPDATE
};

class Message 
{
	string content;
	size_t peer;
};

int main() 
{
	add_regular_command_set();
	set_window_size(200); //fenetre de 200x200
	set_window_resizable(); //fenetre peut etre agrandie
	init();

	/*
	* En gros ca marche plus ou moins de la meme facon que le console renderer
	* juste plus rapide et meilleur
	*/

	int test_variable = 1000;

	std::cout << "int: " << operators::has_operator_equals<int, bool(int)>::value << std::endl;;

	File file("file.txt", FILE_READING_STRING);

	//GLUU::Compiled_ptr gluu_gfx = GLUU::parse_copy(file.getString());

	Server server;

	Random r;

	open_dialog();

	while (run()) //boucle principale
	{
		pencil(COLOR_BLACK); //Permet d'enlever tout ce qui reste de la derniere frame
		draw_clear();

		pencil(rgb(255, 255, 0)); //Couleur jaune

		//draw_line({ 10,0 }, { 0,10 }); //ligne

		//draw_image("Images/chad.jpg", { {20,10},{50,25} }); // dessin de l'image (faut s'assurer que le nom est bon)

		pencil(rainbow(1000)); //arc-en-ciel, boucle de 1000ms

		//draw_circle({ 75,5 }, 5); //cercle

		if (key_pressed(SDL_SCANCODE_SPACE))
			pencil(COLOR_GREEN);
		else
			pencil(COLOR_PINK);

		//draw_full_rect({ 50,50 }); //rectangle plein
		//draw_rect({ {50,102 },50 }); //rectangle pas plein

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

		int pos = (int)(sin((SDL_GetTicks() % 1000) / 100.0) * 100);
		//draw_simple_text("Gluu", { pos,60 }, get_font(1)); //get_font(1) voir 'Fonts/fonts.hint'
		//draw_simple_text("Worked " + strings::stringify(test_variable) + " times", { 0,120 }, get_font(0)); //get_font(0) voir 'Fonts/fonts.hint'

		//Nouvelle fonctionnalité !
		//Entrer la commande 'list all' :)
		track_variable(pos, "pos");
		track_variable(test_variable, "zero_bitches");

		if (key_released(SDL_SCANCODE_1)) 
		{
			//p2p().host();
			//p2p().wait_for_peer();
			server.open_session([](Server& server) 
				{
					map<size_t, ReadBuffer>& buffers = server.get_buffers();

					for (auto& b : buffers)
					{
						ReadBuffer& rb = b.second;
						if (rb.has(0))
						{
							int i1, i2, i3;
							//net.wait_for_stream(0); //On attend que le canal '0' arrive, puis on lit les données
							//net.read_stream(0, i1, i2, i3);
							//server >> i1 >> i2 >> i3;
							rb.read_stream(0, i1, i2, i3);
							std::cout << i1 << " " << i2 << " " << i3 << " " << std::endl;
						}
						if (rb.has(1))
						{
							string s;
							size_t read = rb.read<size_t>(1);
							for (size_t i = 0; i < read; i++)
							{
								char c = rb.read<char>(1);
								s.push_back(c);
							}
							std::cout << s << std::endl;
						}
					}
				});
			p2p().join();
		}

		if (key_released(SDL_SCANCODE_2))
		{
			p2p().join();
		}

		if (key_pressed(SDL_SCANCODE_RETURN))
		{
			keyboard().openTextInput();
			
		}

		draw_text(keyboard().getTextInput(), 500, 0, get_font(0));
		
		if (p2p().is_connected())
		{
			if (key_pressed(SDL_SCANCODE_X))
			{
				int i1 = r.range(0, 100);
				int i2 = r.range(0, 100);
				int i3 = r.range(0, 100);
				p2p(0) << i1 << i2 << i3 << net::send; //on commence un canal '0', puis on envoie les données
				std::cout << i1 << " " << i2 << " " << i3 << " " << std::endl;
			}

			if (key_pressed(SDL_SCANCODE_Z))
			{
				int i1, i2, i3;
				//net.wait_for_stream(0); //On attend que le canal '0' arrive, puis on lit les données
				//net.read_stream(0, i1, i2, i3);
				p2p(0) >> net::wait >> i1 >> i2 >> i3;
				std::cout << i1 << " " << i2 << " " << i3 << " " << std::endl;
			}

			if (key_pressed(SDL_SCANCODE_E))
			{
				
			}

			if (p2p().has_stream(1)) //Une bonne idée de pratique avant de faire un jeu: faire un 'chat' ou les utilisateurs peuvent envoyer des messages entre eux
			{	
				string s;
				//size_t size = p2p().read<size_t>(1);
				size_t size;
				p2p(1) >> size;
				for (size_t i = 0; i < size; i++)
				{
					//s.push_back(p2p().read<char>(1));
					char c;
					p2p(1) >> c;
					s.push_back(c);

				}
				std::cout << s << std::endl;

				//pour entrer du texte sans std::cin
				 // commence la collection de texte entré sur le clavier
				keyboard().getTextInput(); // récupere le texte entré
				
			}
		}

		V2d_i p1 = { 80, 60 };
		V2d_i p2 = { 80, 10 };
		V2d_i p3 = { 50, 30 };

		pencil(COLOR_GREEN);
		draw_line(p1, p2);
		draw_line(p2, p3);
		draw_line(p1, p3);

		draw_full_triangle(p1, p2, p3);
		
		//gluu_gfx->render({ 0,get_logical_size() });
	}

	return 0;
}