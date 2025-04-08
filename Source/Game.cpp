#include "Game.h"

void Game::initVar()
{
	//初始化变量
	this->window = nullptr;

	this->postal_code = MessageQueue::game_code;

	this->current_act_id = 0;
	this->current_scene_id = 0;

	change_window_state_delay = 0;

	window_state = notfullscreen;
}

void Game::initWin()
{
	//初始化窗口
	this->window = new sf::RenderWindow(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "TheDieIsCast",
		sf::Style::Close | sf::Style::Titlebar | sf::Style::Resize | sf::Style::Default);
	//this->window = new sf::RenderWindow(sf::VideoMode::getDesktopMode(), "Game", sf::Style::Fullscreen);
	this->window->setFramerateLimit(FPS);

	std::filesystem::path file_dir;
#ifdef __APPLE__
	file_dir = EXE_DIR / "Resources" / "Icon" / "icon128.png";
#else
	file_dir = EXE_DIR / "Resources" / "Icon" / "icon64.png";
#endif

	this->icon.loadFromFile(file_dir.string());
	this->window->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	this->window->setView(view);
}

void Game::initObject()
{
	page = new MainPage(window);
	scene = nullptr;
}

void Game::pollEvent()
{
	while (this->window->pollEvent(this->ev))
	{
		switch (this->ev.type)
		{
		case sf::Event::Closed:
			if (scene != nullptr)
			{
				save();
				send(MessageQueue::scene_code, "save and close window");
			}
			else
				window->close();
			break;
		case sf::Event::KeyPressed:
 			switch (this->ev.key.code)
			{
			case sf::Keyboard::Escape:
				send(MessageQueue::scene_code, "escape is pressed");
				break;
			case sf::Keyboard::LShift:
			case sf::Keyboard::RShift:
				if (page != nullptr)
					send(page->getPostalCode(), "shift is pressed");
				else if (scene != nullptr)
					send(MessageQueue::scene_code, "shift is pressed");
				break;
			case sf::Keyboard::LControl:
			case sf::Keyboard::RControl:
				if (page != nullptr)
					send(page->getPostalCode(), "control is pressed");
				break;
			case sf::Keyboard::Up:
				if (page != nullptr)
					send(MessageQueue::page_code, "up is pressed");
				else if (scene != nullptr)
					if (scene->canControlPlayer())
						send(MessageQueue::player_code, "up is pressed");
					else
						send(MessageQueue::scene_code, "up is pressed");
				break;
			case sf::Keyboard::Down:
				if (page != nullptr)
					send(MessageQueue::page_code, "down is pressed");
				else if (scene != nullptr)
					if (scene->canControlPlayer())
						send(MessageQueue::player_code, "down is pressed");
					else
						send(MessageQueue::scene_code, "down is pressed");
				break;
			case sf::Keyboard::Left:
				if (page != nullptr)
					send(MessageQueue::page_code, "left is pressed");
				else if (scene != nullptr)
					if (scene->canControlPlayer())
						send(MessageQueue::player_code, "left is pressed");
					else
						send(MessageQueue::scene_code, "left is pressed");
				break;
			case sf::Keyboard::Right:
				if (page != nullptr)
					send(MessageQueue::page_code, "right is pressed");
				else if (scene != nullptr)
					if (scene->canControlPlayer())
						send(MessageQueue::player_code, "right is pressed");
					else
						send(MessageQueue::scene_code, "right is pressed");
				break;
			case sf::Keyboard::Enter:
				if (page != nullptr)
					send(MessageQueue::page_code, "enter is pressed");
				else if (scene != nullptr)
					send(MessageQueue::scene_code, "enter is pressed");
				break;
			case sf::Keyboard::Space:
				if (page != nullptr)
					send(MessageQueue::page_code, "space is pressed");
				else if (scene != nullptr)
					if (scene->canControlPlayer())
						send(MessageQueue::player_code, "space is pressed");
					else
						send(MessageQueue::scene_code, "space is pressed");
				break;
#if SHOW_CONSOLE
			case sf::Keyboard::W:
				if (cheating_mode)
					send(MessageQueue::player_code, "w is pressed");
				else if (scene != nullptr && scene->canControlPlayer())
					send(MessageQueue::player_code, "space is pressed");
				break;
			case sf::Keyboard::S:
				if (cheating_mode)
					send(MessageQueue::player_code, "s is pressed");
				break;
			case sf::Keyboard::A:
				if (scene != nullptr && (cheating_mode || scene->canControlPlayer()))
					send(MessageQueue::player_code, "a is pressed");
				break;
			case sf::Keyboard::D:
				if (scene != nullptr && (cheating_mode || scene->canControlPlayer()))
					send(MessageQueue::player_code, "d is pressed");
				break;
#else
			case sf::Keyboard::W:
				if (scene != nullptr && scene->canControlPlayer())
					send(MessageQueue::player_code, "space is pressed");
				break;
			case sf::Keyboard::A:
				if (scene != nullptr && scene->canControlPlayer())
					send(MessageQueue::player_code, "a is pressed");
				break;
			case sf::Keyboard::D:
				if (scene != nullptr && scene->canControlPlayer())
					send(MessageQueue::player_code, "d is pressed");
				break;
#endif
			case sf::Keyboard::F:
				send(MessageQueue::all_interactivable_code, "f is pressed");
				break;
			case sf::Keyboard::F10:
				if (change_window_state_delay == 0)
				{
					change_window_state_delay = FPS / 2;
					if (window_state == fullscreen)
					{
						window_state = notfullscreen;
						//auto view_copy = this->window->getView();
						//view_copy.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);
						this->window->create(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "TheDieIsCast",
							sf::Style::Close | sf::Style::Titlebar | sf::Style::Resize | sf::Style::Default);
						this->window->setView(view);
						std::filesystem::path file_dir;
#ifdef __APPLE__
						file_dir = EXE_DIR / "Resources" / "Icon" / "icon128.png";
#else
						file_dir = EXE_DIR / "Resources" / "Icon" / "icon64.png";
#endif

						this->icon.loadFromFile(file_dir.string());
						this->window->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
					}
					else
					{
						window_state = fullscreen;
						//auto view_copy = this->window->getView();
						window->create(sf::VideoMode::getDesktopMode(), "TheDieIsCast", sf::Style::Fullscreen);
						//view_copy.setSize({ view_copy.getSize().y * window->getSize().x / window->getSize().y ,
						//	view_copy.getSize().y });
						this->window->setView(view);
					}
				}
				break;
			default:
				break;
			}
			break;
		case sf::Event::KeyReleased:
			switch (this->ev.key.code)
			{
			case sf::Keyboard::LShift:
			case sf::Keyboard::RShift:
				if (page != nullptr)
					send(page->getPostalCode(), "shift is released");
				else if (scene != nullptr)
					send(MessageQueue::scene_code, "shift is released");
				break;
			case sf::Keyboard::RControl:
			case sf::Keyboard::LControl:
#if SHOW_CONSOLE
				if (scene != nullptr)
				{
					cheating_mode = !cheating_mode;
					break;
				}
#endif
				if (page != nullptr)
					send(page->getPostalCode(), "control is released");
				break;
			case sf::Keyboard::Left:
			case sf::Keyboard::A:
				//std::cout << "left walking key released \n";
				if (scene != nullptr)
					send(MessageQueue::player_code, "left walking key released");
				break;
			case sf::Keyboard::Right:
			case sf::Keyboard::D:
				//std::cout << "right walking key released\n";
				if (scene != nullptr)
					send(MessageQueue::player_code, "right walking key released");
				break;
			case sf::Keyboard::Up:
#if !SHOW_CONSOLE
			case sf::Keyboard::W:
#endif
			case sf::Keyboard::Space:
				//std::cout << "jump key released\n";
				if (scene != nullptr && scene->canControlPlayer())
					send(MessageQueue::player_code, "jump key released");
				break;
#if SHOW_CONSOLE
			case sf::Keyboard::RBracket:
				if (cheating_mode && scene != nullptr)
					MessageQueue::send(MessageQueue::scene_code, MessageQueue::game_code, "game completed");
				break;
			case sf::Keyboard::LBracket:
				if (cheating_mode && scene != nullptr && act_scene_recorder.size() > 1)
				{
					act_scene_recorder.pop_back();
					if (current_act_id != act_scene_recorder.back().act_id)
					{
						current_act_id = act_scene_recorder.back().act_id;
						act_in_tree = Script::findAct(script_tree, current_act_id);

						//加载幕信息
						auto file_dir = EXE_DIR / "Script" / act_in_tree.act_file_name;
						Script::parseActScript(file_dir.string(), act);
					}
					else
						current_act_id = act_scene_recorder.back().act_id;

					current_scene_id = act_scene_recorder.back().scene_id;
					if (scene != nullptr)
					{
						delete scene;
						scene = nullptr;
					}
					this->scene = new Scene(act.maps.at(current_scene_id - 1), window);
				}
				break;
			case sf::Keyboard::W:
				if (cheating_mode && scene != nullptr)
					send(MessageQueue::player_code, "w is released");
				else if (scene != nullptr && scene->canControlPlayer())
					send(MessageQueue::player_code, "jump key released");
				break;
			case sf::Keyboard::S:
				if (cheating_mode && scene != nullptr)
					send(MessageQueue::player_code, "s is released");
				break;
#endif
			default:
				break;
			}
			break;
		case sf::Event::LostFocus:
			send(MessageQueue::scene_code, "escape is pressed");
			break;
		default:
			break;
		}
	}

}

Game::Game()
{
	this->initVar();
	this->initWin();
	this->initObject();
}

Game::~Game()
{
	if (page != nullptr)
		delete page;
	if (scene != nullptr)
		delete scene;
	delete window;
}

bool Game::isWindowOpen()
{
	return this->window->isOpen();
}

void Game::update()
{
	if (change_window_state_delay > 0)
		change_window_state_delay--;

	checkMailBoxAndReact();

	this->pollEvent();

	if (page != nullptr)
		page->update(window);

	if (scene != nullptr)
		scene->update();
}

void Game::render()
{
	this->window->clear(sf::Color::Black);

	if (page != nullptr)
		window->draw(*page);
	else if (scene != nullptr)
		scene->render();

	window->display();
}

void Game::waitForClose()
{
	window->setMouseCursorVisible(true);
	while (this->window->pollEvent(this->ev))
	{
		switch (ev.type)
		{
		case sf::Event::KeyPressed:
			switch (this->ev.key.code)
			{
			case sf::Keyboard::F10:
				if (change_window_state_delay == 0)
				{
					change_window_state_delay = FPS / 2;
					if (window_state == fullscreen)
					{
						window_state = notfullscreen;
						//auto view_copy = this->window->getView();
						//view_copy.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);
						this->window->create(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "TheDieIsCast",
							sf::Style::Close | sf::Style::Titlebar | sf::Style::Resize | sf::Style::Default);
						this->window->setView(view);
						std::filesystem::path file_dir;
#ifdef __APPLE__
						file_dir = EXE_DIR / "Resources" / "Icon" / "icon128.png";
#else
						file_dir = EXE_DIR / "Resources" / "Icon" / "icon64.png";
#endif

						this->icon.loadFromFile(file_dir.string());
						this->window->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
					}
					else
					{
						window_state = fullscreen;
						//auto view_copy = this->window->getView();
						window->create(sf::VideoMode::getDesktopMode(), "TheDieIsCast", sf::Style::Fullscreen);
						//view_copy.setSize({ view_copy.getSize().y * window->getSize().x / window->getSize().y ,
						//	view_copy.getSize().y });
						this->window->setView(view);
					}
				}
				break;
			default:
				break;
			}
			break;
		case sf::Event::Closed:
				window->close();
			break;
		default:
			break;
		}
	}

	if (change_window_state_delay > 0)
		change_window_state_delay--;
}

void Game::react()
{
	MessageQueue::Mail mail = check();

	if (mail.from == MessageQueue::blank_code)
		return;

	//筛选来自场景的消息
	if (mail.from == MessageQueue::scene_code)
	{
		if (mail.message == "game completed")
		{
			if (current_scene_id < act.maps.size())
			{
				spark_num = 0;
				points = 0;
				delete this->scene;
				current_scene_id++;

				ResourceManagement::deleteManager();	//重置资源管理器以释放资源
				ResourceManagement::initManager();
				this->scene = new Scene(act.maps.at(current_scene_id - 1), window);
#if SHOW_CONSOLE
				act_scene_recorder.push_back({ current_act_id, current_scene_id });
#endif
			}
			else
			{
				//尝试切换下一个act
				if (act_in_tree.next_acts.size() > 0)
				{
					for (const auto& next_act : act_in_tree.next_acts)
					{
						//检查切换条件
						if (Script::checkEnterCondition(next_act) == true)
						{
							current_act_id = next_act.next_act_id;
							
							act_in_tree = Script::findAct(script_tree, current_act_id);

							//加载幕信息
							auto file_dir = EXE_DIR / "Script" / act_in_tree.act_file_name;
							Script::parseActScript(file_dir.string(), act);
							if (act.maps.size() > 0)
							{
								current_scene_id = act.maps.at(0).map_id;

								ResourceManagement::deleteManager();
								ResourceManagement::initManager();

								if (scene != nullptr)
								{
									delete scene;
									scene = nullptr;
								}

#if SHOW_CONSOLE
								act_scene_recorder.push_back({ current_act_id, current_scene_id });
#endif

								//加载场景
								this->scene = new Scene(act.maps.at(current_scene_id - 1), window);

								break;
							}
						}
					}
				}
				else
				{
					ResourceManagement::deleteManager();
					ResourceManagement::initManager();
					if (scene != nullptr)
					{
						delete scene;
						scene = nullptr;
					}

					page = new EndingPage(window, act_in_tree.end_name);
				}
			}
		}
		else if (mail.message == "back to main page")
		{
			if (scene != nullptr)
			{
				delete scene;
				scene = nullptr;
			}
			page = new MainPage(window);
		}
		else if (mail.message == "close window")
		{
			this->window->close();
		}
	}
	//筛选来自界面的消息
	else if (MessageQueue::computeInterclassicCode(mail.from) == MessageQueue::interface_header)
	{
		if (mail.message == "continue game")
		{
			//删除主界面
			if (page != nullptr)
			{
				delete page;
				page = nullptr;
			}

			//读取脚本
			auto file_dir = EXE_DIR / "Script" / "ScriptTree.json";
			Script::parseScriptTree(file_dir.string(), this->script_tree);

			//读取存档
			load();

			//查找对应的幕
			act_in_tree = Script::findAct(script_tree, current_act_id);

			//加载幕信息
			file_dir = EXE_DIR / "Script" / act_in_tree.act_file_name;
			Script::parseActScript(file_dir.string(), act);
			
			//查找对应的场景
			for (auto& scene : act.maps)
			{
				if (scene.map_id == current_scene_id)
				{
					this->scene = new Scene(scene, window, true);
					return;
				}
			}

			this->scene = new Scene(act.maps.at(0), window);
			return;
		}
		else if (mail.message == "new game")
		{
			//删除主界面
			if (page != nullptr)
			{
				delete page;
				page = nullptr;
			}

			//读取脚本
			auto file_dir = EXE_DIR / "Script" / "ScriptTree.json";
			Script::parseScriptTree(file_dir.string(), this->script_tree);

			//选择第一幕作为起点
			act_in_tree = Script::findAct(script_tree, 1);
			current_act_id = act_in_tree.act_id;

			//加载幕信息
			file_dir = EXE_DIR / "Script" / act_in_tree.act_file_name;
			Script::parseActScript(file_dir.string(), act);

			current_scene_id = act.maps.at(0).map_id;

#if SHOW_CONSOLE
			act_scene_recorder.clear();
			act_scene_recorder.push_back({ current_act_id, current_scene_id });
#endif

			//加载场景
			this->scene = new Scene(act.maps.at(current_scene_id - 1), window);
			return;
		}
		else if (mail.message == "quit")
		{
			this->window->close();
			ResourceManagement::stopBackgroundMusic();
			return;
		}
		else if (mail.message == "save and exit")
		{
			MessageQueue::clear();
			save();
			send(MessageQueue::scene_code, "save and exit");
			return;
		}
	}
	else if (mail.from == MessageQueue::page_code)
	{
		if (mail.message == "back to main page")
		{
			if (page != nullptr)
			{
				delete page;
				page = nullptr;
			}
			page = new MainPage(window);
		}
	}
}

void Game::save()
{
	auto file_dir = EXE_DIR / "History" / "save.bin";
	std::ofstream file(file_dir, std::ios::binary);
	if (!file.is_open()) 
	{
		std::cerr << "failed to open save file" << std::endl;
		return;
	}

	file.write(reinterpret_cast<char*>(&ending_has_achieved), sizeof(ending_has_achieved));
	file.write(reinterpret_cast<char*>(&current_act_id), sizeof(current_act_id));
	file.write(reinterpret_cast<char*>(&current_scene_id), sizeof(current_scene_id));
	file.write(reinterpret_cast<char*>(&spark_num), sizeof(spark_num));
	file.write(reinterpret_cast<char*>(&points), sizeof(points));
	file.write(reinterpret_cast<char*>(&player_respwan_point), sizeof(player_respwan_point));

	file.close();
}

void Game::load()
{
	auto file_dir = EXE_DIR / "History" / "save.bin";
	std::ifstream file(file_dir, std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "failed to open save file" << std::endl;
	}

	file.read(reinterpret_cast<char*>(&ending_has_achieved), sizeof(ending_has_achieved));
	file.read(reinterpret_cast<char*>(&current_act_id), sizeof(current_act_id));
	file.read(reinterpret_cast<char*>(&current_scene_id), sizeof(current_scene_id));
	file.read(reinterpret_cast<char*>(&spark_num), sizeof(spark_num));
	file.read(reinterpret_cast<char*>(&points), sizeof(points));
	file.read(reinterpret_cast<char*>(&player_respwan_point), sizeof(player_respwan_point));

	file.close();
}