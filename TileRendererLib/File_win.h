#pragma once

#pragma once
#include "SDL.h"
#include "Windows.h"
#include <commdlg.h> 
#include "SDL_syswm.h"
#include "Controls.h"

#include <vector>

struct WindowsDialogBox
{
	WindowsDialogBox() {}
	~WindowsDialogBox() {}

	LPCSTR filter = "Images\0*.png\0Any\0*.*\0";
	string dir = "";
	vector<string> paths;

	void open()
	{
		
	}
};