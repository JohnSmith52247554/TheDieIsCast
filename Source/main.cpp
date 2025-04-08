#include "Config.h"
#include "Game.h"
#include "ResourceManagement.h"
#include "ErrorWindow.h"

short map_width_brick;
short map_height_brick;
sf::Vector2i player_respwan_point = { 100, 100 };
unsigned short spark_num = 0;
unsigned int points = 0;
unsigned short ending = 0;
unsigned char ending_has_achieved = 0;
unsigned char minute = 0, second = 0, fps_counter = 0;
std::mt19937 gen;
sf::View view({0, 0, SCREEN_WIDTH * VIEW_SIZE, SCREEN_HEIGHT * VIEW_SIZE });

#if SHOW_CONSOLE
bool cheating_mode = false;
#endif

#ifdef WIN32
const std::filesystem::path EXE_DIR = File::getExecutableDir();
#elif __linux__
const std::filesystem::path EXE_DIR = File::getExecutableDir();
#elif __APPLE__
#if SHOW_CONSOLE
const std::filesystem::path EXE_DIR = File::getExecutableDir();
#else
const std::filesystem::path EXE_DIR = File::getResourceDirectory();
#endif
#endif

#if SHOW_CONSOLE
int main()
#else
#ifdef WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
#else
int main()
#endif
#endif
{
#if ALLOCATION_INDICATOR
	Debug::initialize();
#endif
	Game* game = nullptr;
	MessageQueue::resetPostalCodeCounter();
	ResourceManagement::initManager();

	while (true)
	{
		try
		{
			std::random_device rd;
			gen.seed(rd());

			if (game == nullptr)
				game = new Game;

			sf::Clock clock;
			const float target_frame_time = 1.0f / static_cast<float>(FPS);

			//主循环
			while (game->isWindowOpen() == true)
			{
				clock.restart();

				{
					//Debug::Timer();

					//更新
					game->update();

					//渲染
					game->render();
				}

				ResourceManagement::cleanSounds();

				sf::Time frame_time = clock.getElapsedTime();
				if (frame_time.asSeconds() < target_frame_time)
				{
					sf::Time time_to_sleep = sf::seconds(target_frame_time - frame_time.asSeconds());
					sf::sleep(time_to_sleep);
				}

				//std::cout << "--------\n";

#if ALLOCATION_INDICATOR
				if (Debug::alloc_count > 0 || Debug::alloc_size > 0)
				{
					//std::cout << "new called " << Debug::alloc_count << "\tsize " << Debug::alloc_size << " bytes\n";
					Debug::alloc_count = 0U;
					Debug::alloc_size = 0U;
				}
#endif
			}

			ResourceManagement::deleteManager();
			break;
		}
		catch (const std::runtime_error& e)
		{
			ErrorWindow ew(false, e.what());
			game->update();
			sf::Clock clock;
			const float target_frame_time = 1.0f / static_cast<float>(FPS);
			while (game->isWindowOpen() == true)
			{
				game->waitForClose();
				ew.update();
				if (ew.isWindowOpen() == false)
					break;
				sf::Time frame_time = clock.getElapsedTime();
				if (frame_time.asSeconds() < target_frame_time)
				{
					sf::Time time_to_sleep = sf::seconds(target_frame_time - frame_time.asSeconds());
					sf::sleep(time_to_sleep);
				}
			}
		}
		catch (const std::exception& e)
		{
			sf::Clock clock;
			const float target_frame_time = 1.0f / static_cast<float>(FPS);
			ErrorWindow ew(true, e.what());
			while (game->isWindowOpen() == true)
			{
				if (game->isWindowOpen() == true)
					game->waitForClose();
				if (ew.isWindowOpen() == true)
					ew.update();
				else
					break;
				sf::Time frame_time = clock.getElapsedTime();
				if (frame_time.asSeconds() < target_frame_time)
				{
					sf::Time time_to_sleep = sf::seconds(target_frame_time - frame_time.asSeconds());
					sf::sleep(time_to_sleep);
				}
			}
			break;
		}
	}

	delete game;

#if ALLOCATION_INDICATOR
	Debug::checkMemoryLeaks();
	Debug::shutdown();
#endif

	return 0;
}