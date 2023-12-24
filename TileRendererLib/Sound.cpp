#include "Sound.h"
#include "pch.h"


SoundManager* Sound::manager;

SoundManager::SoundManager() {}

void SoundManager::init()
{
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1) //Initialisation de l'API Mixer
		printf("%s", Mix_GetError());

	Mix_AllocateChannels(8);
	Mix_Volume(1, MIX_MAX_VOLUME / 2);

	std::cout << "Mixer init" << std::endl;
}

Mix_Music* SoundManager::getMusic(std::string path)
{
	if (!loaded_music.count(path))
		loadMusic(path);

	return loaded_music.at(path);
}

 Mix_Chunk* SoundManager::getSound(std::string path)
{
	if (!loaded_sounds.count(path))
		loadSound(path);

	return loaded_sounds.at(path);
}

void SoundManager::loadMusic(std::string path)
{
	Mix_Music* music = Mix_LoadMUS(path.c_str());

	if (!music)
		printf("%s", Mix_GetError());

	loaded_music.emplace(path, music);
}

void SoundManager::playMusic(std::string path, int loopCount)
{
	Mix_PlayMusic(getMusic(path), loopCount);
}

void SoundManager::playMusicFadeIn(std::string path, int fadeinspeed, int loopCount)
{
	Mix_FadeInMusic(getMusic(path), loopCount, fadeinspeed);
}

void SoundManager::update()
{
	if (!isPlayingMusic() && !music_queue.empty())
	{
		playMusicFadeIn(music_queue.front().first, queueFadeIn, music_queue.front().second);
		music_queue.pop_front();
	}
}
