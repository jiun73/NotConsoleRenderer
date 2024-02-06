#include "ludo.h"

int main()
{
	set_window_size(window);
	set_window_resizable();
	setlocale(LC_ALL, "");

	while (run())
	{
		pencil(COLOR_WHITE);
		draw_clear();

		pencil(COLOR_GREEN);
		draw_full_rect(Rect({ {200,120},{200,200} }));
	}
}