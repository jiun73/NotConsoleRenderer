#pragma once
#include "SDL_mixer.h"
#include <iostream>
#include <map>
#include <forward_list>

//Wrapper for SDL_mixer
class SoundManager
{
private:
	std::map<std::string, Mix_Music*> loaded_music;
	std::map<std::string, Mix_Chunk*> loaded_sounds;
	std::forward_list<std::pair<std::string, int>> music_queue;
	int queueFadeIn = 0;

public:
	SoundManager();
	~SoundManager() {}

	void init();

	Mix_Music* getMusic(std::string path);
	Mix_Chunk* getSound(std::string path);

	void loadMusic(std::string path);
	void loadSound(std::string path) 
	{ 
		Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());

		if (!chunk)
		{
			std::cout << "Failed to load " << path << " " << Mix_GetError() << std::endl;;
		}

		loaded_sounds.emplace(path, chunk); 
	}

	void playMusic(std::string path, int loopCount = -1);
	void playMusicFadeIn(std::string path, int fadeinspeed, int loopCount = -1);
	void stopMusic() { Mix_HaltMusic(); }
	void stopMusicFadeOut(int fadeoutspeed) { Mix_FadeOutMusic(fadeoutspeed); }
	void pauseMusic() { Mix_PauseMusic(); }
	void resumeMusic() { Mix_ResumeMusic(); }
	void rewindMusic() { Mix_RewindMusic(); }

	void queueMusic(std::string path, int loopCount = 0) { music_queue.push_front({ path, loopCount }); }
	void setQueueFadeIn(int fadeinspeed) { queueFadeIn = fadeinspeed; }
	void fadeOutTransition(std::string next, int fadeoutspeed) { queueMusic(next), stopMusicFadeOut(fadeoutspeed); }

	bool isPlayingMusic() { return Mix_PlayingMusic(); }

	int channelCount() { return Mix_AllocateChannels(-1); }
	bool playSound(std::string path, int loopCount = 0) { return (Mix_PlayChannel(-1, getSound(path), loopCount) != -1); }
	void pauseSounds() { Mix_Pause(-1); }
	void resumeSounds() { Mix_Resume(-1); }

	void update();
};

class Sound //Sound singleton
{
private:
	static SoundManager* manager;
public:
	static SoundManager* get()
	{
		if (!manager)
			manager = new SoundManager();

		return manager;
	}
};