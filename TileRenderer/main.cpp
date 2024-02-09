
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
	init();

	/*
	* En gros ca marche plus ou moins de la meme facon que le console renderer
	* juste plus rapide et meilleur
	*/

	int test_variable = 0;

	CstarParser parser;
	File file("file.txt", FILE_READING_STRING);
	File file2("file2.txt", FILE_READING_STRING);

	int line1;
	track_variable(line1, "line1");
	int line2;
	track_variable(line2, "line2");

	shared_generic eq_func = std::make_shared<GenericFunctionType<function<void(shared_generic, shared_generic)>>>([](shared_generic b, shared_generic a)
		{
			a->set(b);
		});
	variable_dictionnary()->global()->add(eq_func, "=");

	shared_generic for_func = std::make_shared<GenericFunctionType<function<void(Cstar&, Cstar&)>>>([](Cstar& expr, Cstar& condition)
		{	
			while (true)
			{
				shared_generic gen = condition.evaluate();
				if (gen->type() != typeid(bool)) { std::cout << "wrong type for while" << std::endl;  return; }
				if (!*(bool*)(gen->raw_bytes())) return;

				expr.evaluate();
			}
		});
	variable_dictionnary()->global()->add(for_func, "$while");

	shared_generic if_func = std::make_shared<GenericFunctionType<function<void(Cstar&, bool)>>>([](Cstar& expr, bool b)
		{
			if (b)
			{
				expr.evaluate();
			}
		});
	variable_dictionnary()->global()->add(if_func, "$if");

	shared_generic feach_func = std::make_shared<GenericFunctionType<function<void(Cstar&, shared_generic)>>>([](Cstar& expr, shared_generic a)
		{
			if (a->identity() == typeid(GenericContainer))
			{
				shared_ptr<GenericContainer> container = std::reinterpret_pointer_cast<GenericContainer>(a);
				for (size_t i = 0; i < container->size(); i++)
				{
					expr.evaluate();
				}
			}
			else
				std::cout << "Cannot foreach a non-container" << std::endl;
		});
	variable_dictionnary()->global()->add(feach_func, "$foreach");

	shared_generic add_func = std::make_shared<GenericFunctionType<function<void(int& )>>>([](int& a)
		{
			a++;
		});
	variable_dictionnary()->global()->add(add_func, "++");



	shared_generic cout_func = std::make_shared<GenericFunctionType<function<void(shared_generic)>>>([](shared_generic a)
		{
			std::cout << a->stringify() << std::endl;
		});
	variable_dictionnary()->global()->add(cout_func, ":cout");

	shared_generic grea_func = std::make_shared<GenericFunctionType<function<bool(int, int)>>>([](int b, int a)
		{
			return a > b;
		});
	variable_dictionnary()->global()->add(grea_func, ":gr");

	shared_generic less_func = std::make_shared<GenericFunctionType<function<bool(int, int)>>>([](int b, int a)
		{
			return a < b;
		});
	variable_dictionnary()->global()->add(less_func, ":ls");

	shared_generic destr_func = std::make_shared<GenericFunctionType<function<void(string, shared_generic)>>>([](string b, shared_generic a)
		{
			a->destringify(b);
		});
	variable_dictionnary()->global()->add(destr_func, ":=");

	shared_generic discard_func = std::make_shared<GenericFunctionType<function<void(shared_generic)>>>([](shared_generic a)
		{});
	variable_dictionnary()->global()->add(discard_func, "!#");

	shared_generic space_func = std::make_shared<GenericFunctionType<function<string()>>>([]()
		{
			return " ";
		});
	variable_dictionnary()->global()->add(space_func, "#_");

	shared_generic newline_func = std::make_shared<GenericFunctionType<function<string()>>>([]()
		{
			return "\n";
		});
	variable_dictionnary()->global()->add(newline_func, "#/");

	shared_generic strify_func = std::make_shared<GenericFunctionType<function<string(shared_generic)>>>([](shared_generic a)
		{
			return a->stringify();
		});
	variable_dictionnary()->global()->add(strify_func, "#");

	shared_generic pushb_func = std::make_shared<GenericFunctionType<function<void(shared_generic, shared_generic)>>>([](shared_generic b, shared_generic a)
		{
			if (a->identity() == typeid(GenericContainer))
			{
				shared_ptr<GenericContainer> container = std::reinterpret_pointer_cast<GenericContainer>(a);
				container->insert(b, container->size());
			}
			else
				std::cout << "Cannot push a non-container" << std::endl;
		});
	variable_dictionnary()->global()->add(pushb_func, "-pushb");

	shared_generic pushf_func = std::make_shared<GenericFunctionType<function<void(shared_generic, shared_generic)>>>([](shared_generic b, shared_generic a)
		{
			if (a->identity() == typeid(GenericContainer))
			{
				shared_ptr<GenericContainer> container = std::reinterpret_pointer_cast<GenericContainer>(a);
				container->insert(b, 0);
			}
			else
				std::cout << "Cannot push a non-container" << std::endl;
		});
	variable_dictionnary()->global()->add(pushf_func, "-pushf");

	shared_generic insert_func = std::make_shared<GenericFunctionType<function<void(int, shared_generic, shared_generic)>>>([](size_t i, shared_generic b, shared_generic a)
		{
			if (a->identity() == typeid(GenericContainer))
			{
				shared_ptr<GenericContainer> container = std::reinterpret_pointer_cast<GenericContainer>(a);
				container->insert(b, i);
			}
			else
				std::cout << "Cannot push a non-container" << std::endl;
		});
	variable_dictionnary()->global()->add(insert_func, "-ins");

	shared_generic at_func = std::make_shared<GenericFunctionType<function<shared_generic(int, shared_generic)>>>([](size_t i, shared_generic a) -> shared_generic
		{
			if (a->identity() == typeid(GenericContainer))
			{
				shared_ptr<GenericContainer> container = std::reinterpret_pointer_cast<GenericContainer>(a);
				return container->at(i);
			}
			else
				std::cout << "Cannot at a non-container" << std::endl;
			return nullptr;
		});
	variable_dictionnary()->global()->add(at_func, "-at");

	string str = file.getString();
	string str2 = file2.getString();
	parser.parse(str);
	//shared_generic gen = parser.parse_sequence(str2).evaluate();
	//std::cout << gen->type().name() <<  "\n" << gen->stringify() << std::endl;

	Server server;

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

		if (!keyboard().getTextInput().empty() && keyboard().getTextInput().back() == '\n')
		{
			
			//p2p().start_stream(1); //BTW on peut pas envoyer directement des conteneurs (vector, string, etc) parce qu'ils ne contiennent pas vraiment les données, mais plutot des pointers VERS les données (qui ne seront pas valide sur lordinateur de lautre)
			//p2p().send(s.size()); //on envoie la taille de la string
			//for (auto& c : s)
			//	p2p().send(c); //puis on envoie chaque caratère un a la fois
			//p2p().end_stream();
			const string& s = keyboard().getTextInput();
			p2p(1) << s.size();
			for (auto& c : s)
				p2p(1) << c;
			p2p(1) << net::send;

			keyboard().getTextInput() = "";

			keyboard().closeTextInput();
		}
		
		if (p2p().is_connected())
		{
			if (key_pressed(SDL_SCANCODE_X))
			{
				int i1 = r.range(0, 100);
				int i2 = r.range(0, 100);
				int i3 = r.range(0, 100);
				//p2p().start_stream(0); //on commence un canal '0', puis on envoie les données
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
	}

	return 0;
}