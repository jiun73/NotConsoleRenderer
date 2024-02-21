#pragma once

#include "SDL.h"
#include "Input.h"
#include "Color.h"
#include "Random.h"
#include "Rect.h"
#include "Sound.h"
#include "Fonts.h"
#include "Camera.h"
#include "CommandDictionnary.h"
#include "Stringify.h"
#include "Controls.h"

#undef main	

using std::pair;

/*
* C'est ici qu'il y a tout les fonction que tu peut appeller avec mon engine
*/

//Defois ta des probleme que 'cannot open file TileRenderer.h' ben faut que tu build TileRendererLib sur visualstuidio

//----------------------------------------------------------FENETRE---------------------------------------------------------------------------

//Rend la fenêtre de l'application 'resizable' lors de la création (donc tu peut changer la taille)
//À appeler AVANT 'run'
void set_window_resizable();

//Définit la taille de la fenêtre lors de sa création 
//Si la fenêtre est ensuite agrandi/rétréssi, la taille ne change pas, mais les pixels deviennent plus gros
//À appeler AVANT 'run'
void set_window_size(V2d_i size);

//Retourne le 'scale' par laquelle l'écran est multiplé
//Par exemple, un écran qui est agrandi au double de sa taille originelle retourne 2
//Correspond, si on veut, à la taille d'un pixel
V2d_d get_renderer_scale();

//Retourne la taille présente de la fenêtre 
V2d_d get_window_size();

V2d_d get_logical_size();

//----------------------------------------------------------SON-------------------------------------------------------------------------------

//Retourne simplement une référence vers le SoundManager, voir la définition de la classe dans Sound.h
SoundManager& sound();

//----------------------------------------------------------PRGORAMME-------------------------------------------------------------------------

//Initialise tout et crée la fenêtre
//Certaines fonction requiert qu'elle soit appellé après celle-ci pour bien fonctionner
//Si elle n'est pas appellé manuellement, run() l'appelle
void init();

//Ferme le programme
//À noter que le fermeture se fait simplement à la fin d'une boucle principale
void close();

//Boucle principale
//À mettre dans un 'while'
//Est appellé maximum 60 fois par secondes
bool run();

//----------------------------------------------------------CONTROLES-------------------------------------------------------------------------

MultiInput& inputs();
KeyboardInput& keyboard();
MouseInput& mouse();
JoystickInput& joystick(); //Oui, tu peut utiliser une manette pour controller, par contre j'ai la flemme de faire des fonction pour

bool key_pressed(SDL_Scancode code); //List des codes : https://wiki.libsdl.org/SDL2/SDL_Scancode SDL_SCANCODE_...
bool key_held(SDL_Scancode code);
bool key_released(SDL_Scancode code);

bool mouse_left_pressed();
bool mouse_left_held();
bool mouse_left_released();

bool mouse_right_pressed();
bool mouse_right_held();
bool mouse_right_released();

bool input(const string& str);


//Retourne la position de la souris prenant en compte l'aggrandissement de la fenêtre
V2d_d mouse_position();

V2d_d game_mouse_position();

//----------------------------------------------------------DESSIN----------------------------------------------------------------------------

namespace hidden { extern Camera _camera; }

//Permet de modifier ou les objects apparaissent sur l'ecran
//À utiliser en tandem avec to_game
Camera& camera();

//Convertit des positions absolue vers des positions relatives à la camera
template<typename T> inline Vector2D<T> to_game(const Vector2D<T>& position){return hidden::_camera.find(position);}
template<typename T> inline vRectangle<T> to_game(const vRectangle<T>& rect) { return hidden::_camera.find(rect); }

//Retourne une couleur rgb
Color rgb(int red, int green, int blue);

//retourne une couleur arc-en-ciel qui change selon le temps
Color rainbow(int speed, int delay = 0);

//Change la couleur de dessin
void pencil(Color color);

//Dessine un simple pixel
void draw_pix(V2d_i position);

//Dessine un rectangle, non rempli
void draw_rect(Rect rectangle);

//Dessine un rectangle plein
void draw_full_rect(Rect rectangle);

//Dessine un ligne allant du point 'start_position' à 'end_position'
void draw_line(V2d_i start_position, V2d_i end_position);

//Dessine un image au rectangle 'destination' donné
//Donc c'est comme dessiner un rectangle, sauf qu'il y a l'image dessus
//'path' est le chemin ou se trouve l'image
void draw_image(const string& path, Rect destination);

//Comme draw_image, sauf que ca permet de dessiner seulement une partie de l'image
//La 'source' est les coordonnés ou va etre pris la partie de l'image a dessiner
//Faut essayer pour comprendre vraiment, c'est difficile a expliquer comme ca
void draw_image_from_source(const string& path, Rect source, Rect destination);

//Dessine un cercle avec le millieu à la position 'pos' et au rayon 'radius'
void draw_circle(V2d_i pos, int radius);

//Dessine une couleur sur l'écran au complet
void draw_clear();

//Permet de charger un texture à l'avance, mais 'draw_image' le fait si ce n'est pas fait manuellement
//\param 'path' est le chemin ou se trouve l'image
void load_texture(const string& path);
V2d_i get_image_size(const string& path);
void output_texture_pixels(const string path);

//----------------------------------------------------------TEXTE-----------------------------------------------------------------------------

FontsManager& fonts(); 
void add_font_effect(const string& name, FontEffect_f effect);

/*
* Charge une police d'ecriture à partir d'un chemin
* La police peut être de plusieurs type (ex: .ttf)
* Une bonne source de ce genre de polices: https://www.dafont.com/fr/theme.php?cat=601&page=2
* 
* Un autre facon de charger une police d'écriture est d'écrire le chemin dans le fichier 'Fonts/fonts.hint'
* de cette facon: 'chemin;taille' (une police par ligne)
* Son index devient alors sa position dans le fichier (ex: ligne 1 = 0)
*/
FontID load_font(const string& path);

/*
* Retourne une police d'écriture par son index
* \param 'i' est un index retourné par load_font() ou 
* la ligne ou la police apparait dans le fichier 'Fonts/fonts.hint'
*/
const Font& get_font(FontID i);
void set_font_pencil(const Color& color, const Font& font);

int draw_glyph(const char& character, const V2d_i& pos, const Font& font, int kerning = 0);

/*
* Dessine du simple texte aligné vers la gauche. supporte les retours de ligne
* \param 'font' est une police d'écriture obtenu avec get_font()
* \param 'pos' est le coin en haut à gauche du texte
*/
void draw_simple_text(const string& text, const V2d_i& pos, const Font& font);

/*
* Dessine du texte aligné vers la gauche. Fait un retour de ligne automatiquement quand 
* la taille maximale est atteint (par contre, ca le fait en verifiant la taille des lettres et non des mots, donc ceu-ci peuvetn etre brisé)
* \param 'text' texte à dessiner
* \param 'max_width' largeur maximale du champ de texte
* \param 'pos' est le coin en haut à gauche du texte
* \param 'font' est une police d'écriture obtenu avec get_font()
*/
void draw_text(const string& text, const int& max_width, const V2d_i& pos, const Font& font);
void draw_special_text(const string& text, const int& max_width, const V2d_i& pos, const Font& font);

/*
* Dessine du simple texte aligné vers la gauche. supporte pas les retours de ligne 
* \param 'font' est une police d'écriture obtenu avec get_font()
* \param 'pos' est le coin en haut à gauche du texte
*/
void draw_line(const string& text, const V2d_i& pos, const Font& font);

/*
* Retourne la taille que prendrait le texte si il est dessiné sur une ligne continue
*/
int get_text_draw_size(const string& text, const Font& font);

typedef pair<string::const_iterator, string::const_iterator> string_range;
#include <functional>

namespace hidden 
{

	template<typename... Ts>
    inline vector<string_range> all_ranges(string_range range, string_range(range_func)(string_range, Ts...), Ts... extras) 
	{
		vector<string_range> ret;

		auto current = range.first;
		while (current != range.second)
		{
			auto sub_range = range_func({current, range.second}, extras...);

			ret.push_back(sub_range);
			
			current = sub_range.second;
		}

		return ret;
	}

	int get_text_draw_size(string::const_iterator begin, string::const_iterator end, const Font& font);
	string_range get_text_range_max_size(string_range range, int max_size, const Font& font);
	string_range get_text_range_until(string_range range, char c);
	string_range get_text_range_special(string_range range, vector<bool>& is_special);
	void draw_line_range(string::const_iterator begin, string::const_iterator end, TextDrawCounter& counter, const Font& font);
}

//----------------------------------------------------------ALEATOIRE-------------------------------------------------------------------------

//retourne un 'Random' (Voir la classe dans Random.h)
Random& random();

//----------------------------------------------------------COLLISION-------------------------------------------------------------------------

bool point_in_rectangle(const V2d_i& point, const Rect& rectangle);

//----------------------------------------------------------UTILITAIRE------------------------------------------------------------------------

double lerp(double a, double b, double t); //Linear interpolation formula
double distance_square(const V2d_d& pos1, const V2d_d& pos2); //square of the distance between two points (to avoid using expensive sqrt)
double distance(const V2d_d& pos1, const V2d_d& pos2); //real distance between points

//----------------------------------------------------------ENTITYX---------------------------------------------------------------------------
#include "EntityX.h"

inline EntityManagerX* entities() { return EntX::get(); }

template<typename T>
inline T* entitity_system() { return EntX::get()->get_system<T>(); }

//----------------------------------------------------------NETWORKING------------------------------------------------------------------------
#include "Networking.h"

Peer2Peer& p2p();
Peer2Peer& p2p(size_t i);
