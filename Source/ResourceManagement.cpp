#include "ResourceManagement.h"

sf::Music ResourceManagement::music;
std::vector<std::unique_ptr<sf::Sound>> ResourceManagement::sound_effects;

std::vector<std::shared_ptr<sf::Texture>> ResourceManagement::texture_list;

std::vector<std::shared_ptr<sf::Font>> ResourceManagement::font_list;

std::vector<std::string> ResourceManagement::texture_dir =
{
	"PlayerStand",	//0
	"PlayerJump",	//1
	"PlayerWalk",	//2
	"PlayerFail",	//3
	"RobotRun",		//4
	"RobotDie",		//5
	"Spark",		//6
	"Button",		//7
	"Wall",			//8
	"SparkTube",	//9
	"Drawbar",		//10
	"PressurePlate",//11
	"LaserGun",		//12
	"Laser",		//13
	"Helmet",		//14
	"RobotFactory", //15
	"ShadowMachine",//16
	"Shadow",		//17
	"Heroine",		//18
	"Villain"		//19
};

std::vector<std::string> ResourceManagement::font_dir =
{
	"DottedSongtiSquareRegular.otf",//0
};

std::unordered_map<std::string, std::shared_ptr<sf::SoundBuffer>> ResourceManagement::sounds_buffers;

void ResourceManagement::initManager()
{
	// 为每个目录名称预留一个可选的shared_ptr
	texture_list.resize(texture_dir.size());

	for (auto& tp : texture_list)
	{
		tp = nullptr;
	}

	font_list.resize(font_dir.size());

	for (auto& fp : font_list)
	{
		fp = nullptr;
	}

	music.setLoop(true);
}

void ResourceManagement::deleteManager()
{
	texture_list.clear();
	font_list.clear();
	// 使用智能指针后，只需清空vector，内存会自动管理

	stopSounds();

	for (auto& pair : sounds_buffers)
		pair.second = nullptr;
	sounds_buffers.clear();

	music.stop();
}

std::shared_ptr<sf::Texture> ResourceManagement::getTexture(int ID)
{
	if(ID < 0 || ID >= texture_list.size())
	{
		std::cout << "Texture ID out of range, ID: " << ID << std::endl;
		throw std::runtime_error("Texture ID out of range, ID");
		return nullptr;
	}

	if (texture_list[ID] == nullptr)
	{
		auto texture = std::make_shared<sf::Texture>();
		auto file_dir = EXE_DIR / "Resources" / "Images" / (texture_dir[ID] + ".png");
		if (!texture->loadFromFile(file_dir.string()))
		{
			std::string error = "Failed to load texture: " + file_dir.string();
			std::cout << error << std::endl;
			throw std::runtime_error(error);
			return nullptr;
		}
		texture_list[ID] = texture;
	}

	return texture_list.at(ID);
}

std::shared_ptr<sf::Font> ResourceManagement::getFont(int ID)
{
	if (ID < 0 || ID >= font_list.size())
	{
		std::string error = "Font ID out of range, ID: " + ID;
		std::cout << error << std::endl;
		throw std::runtime_error(error);
		return nullptr;
	}

	if (font_list[ID] == nullptr)
	{
		auto font = std::make_shared<sf::Font>();
		auto file_dir = EXE_DIR / "Resources" / "Fonts" / font_dir[ID];
		if (!font->loadFromFile(file_dir.string()))
		{
			std::string error = "Failed to load font: " + file_dir.string();
			std::cout << error << std::endl;
			throw std::runtime_error(error);
			return nullptr;
		}
		font_list[ID] = font;
	}

	return font_list.at(ID);
}

void ResourceManagement::setBackgroundMusic(std::string file_name)
{
	auto file_dir = EXE_DIR / "Resources" / "Music" / (file_name + ".mp3");

	if (music.openFromFile(file_dir.string()) == false)
	{
		std::string error = "open music " + file_dir.string() + " failed";
		std::cerr << error << std::endl;
		throw std::runtime_error(error);
	}

	music.play();

	music.setVolume(100.f);
}

void ResourceManagement::stopBackgroundMusic()
{
	music.stop();
}

void ResourceManagement::pauseBackgroundMusic()
{
	music.pause();
}

void ResourceManagement::playBackgroundMusic()
{
	music.play();
}

void ResourceManagement::setBackgroundMusicVolume(const float volume)
{
	music.setVolume(volume);
}

const float ResourceManagement::getBackgroundMusicVolume()
{
	return music.getVolume();
}

void ResourceManagement::checkState()
{
	std::cout << music.getStatus() << "  "
		<< music.getVolume() << std::endl;
}

void ResourceManagement::cleanSounds()
{
	for (auto sound = sound_effects.begin(); sound != sound_effects.end();)
	{
		if ((*sound)->getStatus() == sf::Sound::Stopped)
		{
			sound = sound_effects.erase(sound);
		}
		else
		{
			sound++;
		}
	}
}

void ResourceManagement::playSound(std::string sound_name)
{
	auto& buffer = sounds_buffers[sound_name];

	if (buffer == nullptr)
		buffer = std::make_shared<sf::SoundBuffer>();

	if (buffer->getSampleCount() == 0)
	{
		auto file_dir = EXE_DIR / "Resources" / "Sounds" / (sound_name + ".ogg");
		if (buffer->loadFromFile(file_dir.string()) == false)
		{
			std::string error = "open sound " + file_dir.string() + " failed";
			std::cerr << error << std::endl;
			throw std::runtime_error(error);
		}

	}

	std::unique_ptr<sf::Sound> sound = std::make_unique<sf::Sound>();
	sound->setBuffer(*buffer);
	sound->setVolume(70.f);

	sound->play();
	sound_effects.push_back(std::move(sound));
}

void ResourceManagement::stopSounds()
{
	if (sound_effects.size() == 0)
		return;

	for (auto& sound : sound_effects)
		sound->stop();

	sound_effects.clear();
}