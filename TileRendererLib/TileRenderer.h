#pragma once

#include "SDL.h"
#include "Input.h"
#include "Color.h"
#include "Random.h"
#include "Rect.h"
#include "Sound.h"

#undef main	

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

//Retourne la position de la souris prenant en compte l'aggrandissement de la fenêtre
V2d_d mouse_position();

//----------------------------------------------------------DESSIN----------------------------------------------------------------------------

//Retourne une couleur rgb
Color rgb(int red, int green, int blue);

//retourne une couleur arc-en-ciel qui change selon le temps
Color rainbow(int speed );

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

//Dessine un cercle avec le millieu à la position 'pos' et au rayon 'radius'
void draw_circle(V2d_i pos, int radius);

//Dessine une couleur sur l'écran au complet
void draw_clear();

//Permet de charger un texture à l'avance, mais 'draw_image' le fait si ce n'est pas fait manuellement
//'path' est le chemin ou se trouve l'image
void load_texture(const string& path);

//----------------------------------------------------------ALEATOIRE-------------------------------------------------------------------------

//retourne un 'Random' (Voir la classe dans Random.h)
Random& random();
