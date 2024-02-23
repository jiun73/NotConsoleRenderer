#pragma once
#include "CommandPrompt.h"
#include "CommandDictionnary.h"
#include "ConsoleRenderer.h"

#include "GLUU.h"

inline void add_regular_command_set()
{
	__NEW_COMMAND__("test", [](__COMMAND_ARGS__)
		{
			std::cout << "You tested alright!" << std::endl;
		});

	__NEW_COMMAND__("help", [](__COMMAND_ARGS__)
		{
			std::cout << "This is the command center!" << std::endl;
			std::cout << "you can enter commands or define your own!" << std::endl;
			std::cout << "This is somewhat useful if you want to peek quickly at some variables or something" << std::endl;
			std::cout << "Or maybe you need to setup a specific situation for bug testing... ?" << std::endl;
			std::cout << "Idk you do what you what!" << std::endl;
			std::cout << "special: *** means that the argument can be any string" << std::endl;
			std::cout << "special: [...] means that the argument has additionnal (and optionnal) arguments at the end" << std::endl;
			std::cout << "special: (int) means... huh... let me check..." << std::endl;
		});

	__NEW_COMMAND__("list all", [](__COMMAND_ARGS__)
		{
			for (auto& reg : variable_dictionnary()->all_saved())
			{
				std::cout << "\n--Scope " << reg->name << "--\n\n";
				for (auto& e : reg->all())
					std::cout << e.second->type().name() << " " << e.first << " = " << e.second->stringify() << std::endl;
			}
		});

	__NEW_COMMAND__("list global", [](__COMMAND_ARGS__)
		{
			for (auto& e : variable_dictionnary()->global()->all())
				std::cout << e.second->type().name() << " " << e.first << " = " << e.second->stringify() << std::endl;
		});

	__NEW_COMMAND__("! *** = ***", [&](__COMMAND_ARGS__)
		{
			std::cout << variable_dictionnary()->get(args[1])->destringify(args[3]) << std::endl;
			std::cout << variable_dictionnary()->get(args[1])->stringify() << std::endl;

		});

	__NEW_COMMAND__("GLUU ***", [&](__COMMAND_ARGS__)
		{
			GLUU::parser()->parse_new_sequence(args[1]).evaluate();
		});

	DefineCommand ff4("pause", [](__COMMAND_ARGS__)
		{
			int* b = new int(0);

			CommandSpace space(":");

			space.temp.push_back({ "step (int)", [b](CommandArguments& args)
				{
					*b = args(2);
				} });

			space.temp.push_back({ "close", [b, space](__COMMAND_ARGS__)
				{

					Commands::get()->call_once.push_back([b, space]()
						{
							std::cout << "Unpausing..." << std::endl;
							delete b;
							Commands::get()->exit_space(space);
						});
				} });

			space.callback = [b]()
				{
					std::cout << "Pausing... " << *b << std::endl;
					auto& call_ref = Commands::get()->temp_spaces.at(":").callback;
					auto old = call_ref;
					call_ref = nullptr;
					if (*b == 0)
						Commands::get()->update(true);
					CommandPrompt* p = Commands::get();
					while (*b == 0)
					{
						Commands::get()->update();
						if (!Commands::get()->temp_spaces.count(":")) return;
					};

					call_ref = old;
					(*b)--;

					if (*b < 0)
						*b = 0;
				};

			Commands::get()->enter_space(space);
		});

	DefineCommand _ff_cmd("open watch", [&](CommandArguments& args)
		{
			//PROCESS_INFORMATION processInformation;
			//STARTUPINFO         startupInfo;
			////wchar_t  lpAppName =;

			//ZeroMemory(&startupInfo, sizeof(startupInfo));
			//startupInfo.cb = sizeof(startupInfo);

			//BOOL creationResult = CreateProcess(
			//	L"c:\\windows\\system32\\cmd.exe",                   // No module name (use command line)
			//	NULL,                // Command line
			//	NULL,                   // Process handle not inheritable
			//	NULL,                   // Thread handle not inheritable
			//	FALSE,                  // Set handle inheritance to FALSE
			//	CREATE_NEW_CONSOLE , // creation flags
			//	NULL,                   // Use parent's environment block
			//	NULL,                   // Use parent's starting directory 
			//	&startupInfo,           // Pointer to STARTUPINFO structure
			//	&processInformation);   // Pointer to PROCESS_INFORMATION structure

			//FreeConsole();
			//AttachConsole(processInformation.dwProcessId);

			//assert(false);
			//std::cout << creationResult << std::endl;

			vector<std::pair<string, shared_generic>>* watching = new vector<std::pair<string, shared_generic>>();

			CommandSpace space("watch");

			space.temp.push_back({ "set ***", [watching](__COMMAND_ARGS__)
				{
					string name = args[1];
					shared_generic ptr = variable_dictionnary()->get(name);
					if (ptr != nullptr)
						watching->push_back({ name,ptr });
					else
						std::cout << "couldn't find the variable lol" << std::endl;
				} });

			space.temp.push_back({ "all", [watching](__COMMAND_ARGS__)
				{
					for (auto& reg : variable_dictionnary()->all())
					{
						for (auto& var : reg->all())
						{
							watching->push_back({var.first, var.second});
						}
					}
				} });

			space.temp.push_back({ "close", [watching, space](__COMMAND_ARGS__)
				{
					Commands::get()->call_once.push_back([space]()
						{
							Commands::get()->exit_space(space);
						});

				} });

			space.callback = [watching]()
				{
					std::string output;

					if (watching->size() == 0)
					{
						output = "Nothing to watch :(\n";
					}
					else
					{
						output = "Watching \\/ \n";
					}

					for (auto& w : *watching)
					{
						output += w.first + " = " + w.second->stringify() + "\n";
					}



					auto& cApp = *CApp::get();
					V2d_i old = cApp.getCursor();
					cApp.pencil(BLACK, BG_WHITE, '#');
					cApp.text(output, { 50,0 });
					cApp.color(WHITE, BG_BLACK);
					cApp.present();
					cApp.color(WHITE, BG_BLACK);
					cApp.setCursor(old.xi());

				};

			Commands::get()->enter_space(space);
		});
}