#pragma once

#include "SDL.h"
#include "Input.h"
#include "Color.h"
#include "Random.h"
#include "Rect.h"

#undef main	

void set_window_resizable();
void set_window_size(V2d_i size);

void close();
bool run();

KeyboardInput& keyboard();
MouseInput& mouse();
JoystickInput& joystick();

Color rgb(int red, int green, int blue);
Color rainbow(int speed );

void pencil(Color color);
void draw_pix(V2d_i position);
void draw_rect(Rect rectangle);
void draw_full_rect(Rect rectangle);
void draw_line(V2d_i start_position, V2d_i end_position);
void draw_image(const string& path, Rect destination);
void draw_circle(V2d_i pos, int radius);

void load_texture(const string& path);
Random& random();
