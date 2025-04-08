#pragma once

#include "Config.h"
#include "File.h"

#include <memory>
#include <filesystem>
#include <stdexcept>
#include <unordered_map>

class ResourceManagement
{
private:
	static sf::Music music;
	static std::vector<std::unique_ptr<sf::Sound>> sound_effects;	//可以同时播放多个音效

	static std::vector<std::shared_ptr<sf::Texture>>texture_list;
	static std::vector<std::shared_ptr<sf::Font>> font_list;
	static std::vector<std::string> texture_dir;
	static std::vector<std::string> font_dir;
	static std::unordered_map<std::string, std::shared_ptr<sf::SoundBuffer>> sounds_buffers;

public:
	static void initManager();
	static void deleteManager();

	static std::shared_ptr<sf::Texture> getTexture(int ID);
	static std::shared_ptr<sf::Font> getFont(int ID);

	static void setBackgroundMusic(std::string file_name);
	static void stopBackgroundMusic();
	static void pauseBackgroundMusic();
	static void playBackgroundMusic();
	static void setBackgroundMusicVolume(const float volume);
	static const float getBackgroundMusicVolume();
	static void checkState();

	static void cleanSounds();
	static void playSound(std::string sound_name);
	static void stopSounds();
};

