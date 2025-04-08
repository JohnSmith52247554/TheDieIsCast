#include "Scene.h"

void Scene::initVar()
{
	//初始化变量
	this->window = nullptr;
	/*this->view = nullptr;*/
	this->object_manager = nullptr;
	this->text = nullptr;
	this->time_manager = nullptr;
	this->bar = nullptr;
	this->i_interface = nullptr;
	this->dialogue_interface = nullptr;
	this->player = nullptr;
	this->full_screen_quad = nullptr;
	this->background = nullptr;
	this->background_texture = nullptr;
	this->press_f_reminder = nullptr;

	this->endOfGame = false;
	this->is_rewinding_time = false;
	this->loading_shader = false;
	this->game_pause = false;
	this->show_interface = false;

	this->postal_code = MessageQueue::scene_code;

	this->begin_end_counter = MAP_DESCRIPTION_SHOW_TIME;

	this->state = 0;

	this->npc_info.current_speaking_npc_id = 0;
	this->npc_info.current_speaking_npc_postal_code = MessageQueue::blank_code;

	this->has_player_failed = false;

	this->earthquake = true;
	this->random_quake_counter = 0U;
	this->play_quake_music = false;
	this->camera_shake_counter = 0U;
	this->shake_direction = false;

	final.width = 0;
	final.height = 0;
	final.top = 0;
	final.left = 0;

	second = 0;
	minute = 0;
	fps_counter = 0;

	auto_rewind_time_counter = 0;

	camera_shake_counter = 0U;

#if SHOW_CONSOLE
	cheating_mode_reminder = nullptr;
#endif

	move_camera_speed = 0.f;

	force_rewind = false;
}

void Scene::initWin(sf::RenderWindow* i_render_window)
{
	//初始化窗口
	//this->view = new sf::View(sf::FloatRect(0, 0, SCREEN_WIDTH * VIEW_SIZE, SCREEN_HEIGHT * VIEW_SIZE));
	this->window = i_render_window;
	//this->window->setView(*view);

	// 隐藏鼠标光标
	this->window->setMouseCursorVisible(false);

	// 加载着色器
	auto shader_dir = EXE_DIR / "Resources" / "Shader" / "rewinding.frag";
	if (!shader.loadFromFile(shader_dir.string(), sf::Shader::Fragment))
	{
		std::cerr << "Failed to load shader" << std::endl;
		shader_available = false;
	}
	else
	{
		shader_available = shader.isAvailable();

		if (shader_available)
		{
			shader.setUniform("useShader", false);
			shader.setUniform("fadeFactor", 0.35f);
			shader.setUniform("rgbIntensity", 0.04f);
			shader.setUniform("rgbDirection", sf::Glsl::Vec2(0.5, 0.25));
			shader.setUniform("distortionFactor", 0.2f);
			states.shader = &shader;
		}
	}

	render_texture.create(SCREEN_WIDTH * 4, SCREEN_HEIGHT * 4);
	screen_sprite.setScale(0.25, 0.25);
	render_texture.setView(view);

	auto shining_shader_dir = EXE_DIR / "Resources" / "Shader" / "shining.frag";
	if (!shader2.loadFromFile(shining_shader_dir.string(), sf::Shader::Fragment))
	{
		std::cerr << "Failed to load shader " << shining_shader_dir << std::endl;
	}
	else if (shader_available)
	{
		states2.shader = &shader2;
	}
}

void Scene::initObject(std::string i_map_name)
{
	setDialogueTrigger();

	this->object_manager = new ObjectManagement;

	map.loadFromFile(i_map_name, object_manager);

	this->player = new Player(player_respwan_point.x, player_respwan_point.y, object_manager);

	map.spawnObject(object_manager, script_map.name, map_note, final, player);

	player->setPosition(player_respwan_point.x, player_respwan_point.y);

	if (script_map.description != L"")
	{
		this->text = new sf::Text;	//展示地图描述
		this->text->setFont(*ResourceManagement::getFont(0));
		this->text->setString(script_map.description);
		this->text->setCharacterSize(30 * 4);
		this->text->setScale(0.25, 0.25);
		this->text->setFillColor(sf::Color::White);
		this->text->setPosition(SCREEN_WIDTH / 2 - this->text->getGlobalBounds().width / 2, SCREEN_HEIGHT / 2 - this->text->getGlobalBounds().height / 2 - 6);
	}
	else
	{
		ResourceManagement::setBackgroundMusic(script_map.bgm);
		//设置视角
		if (script_map.move_camera == false)
			setViewToPlayer();
		else
			setViewToTheOtherSide();
	}

	this->time_manager = new TimeManagement;

	this->full_screen_quad = nullptr;

	this->bar = new Bar;

	this->i_interface = nullptr;
	this->dialogue_interface = nullptr;
}

void Scene::setView(sf::Vector2f coord)
{
	view.setCenter(coord);
	this->window->setView(view);
}

void Scene::react()
{
	MessageQueue::Mail mail = check();

	if (mail.from == MessageQueue::blank_code)
		return;

	//筛选出来自游戏的消息
	if (mail.from == MessageQueue::game_code)
	{
		if (mail.message == "up is pressed" || mail.message == "down is pressed" 
			|| mail.message == "left is pressed" || mail.message == "right is pressed"
			|| mail.message == "space is pressed" || mail.message == "enter is pressed")
		{
			if (i_interface != nullptr)
				send(i_interface->getPostalCode(), mail.message);
			else if (dialogue_interface != nullptr)
				send(dialogue_interface->getPostalCode(), mail.message);
			/*else if (player != nullptr)
				send(MessageQueue::player_code, mail.message);*/
		}
		/*else if (mail.message == "a is pressed" || mail.message == "d is pressed")
		{
			if (player != nullptr)
				send(MessageQueue::player_code, mail.message);
		}*/
		else if (mail.message == "escape is pressed")
		{
			// 恢复鼠标光标
			this->show_interface = true;
			this->window->setMouseCursorVisible(true);
			if (game_pause == false)
			{
				if (i_interface != nullptr)
					delete i_interface;
				this->i_interface = new PauseInterface;
				game_pause = true;
			}

			//暂停音乐
			ResourceManagement::pauseBackgroundMusic();

			player->stop();
			//send(MessageQueue::player_code, "stop all motion");
		}
		else if (mail.message == "shift is pressed")
		{
			if (is_rewinding_time == false && dialogue_interface == nullptr && state <= 1  && script_map.be_able_to_rewind_time)
			{
				is_rewinding_time = true;
				player->stop();
				//send(MessageQueue::player_code, "stop all motion");
				ResourceManagement::setBackgroundMusic("II_Reverted");
			}
		}
		else if (mail.message == "shift is released")
		{
			if (is_rewinding_time)
			{
				is_rewinding_time = false;
				player->stop();
				//send(MessageQueue::player_code, "stop all motion");
				if (!earthquake)
					ResourceManagement::setBackgroundMusic(script_map.bgm);
				else
					ResourceManagement::setBackgroundMusic("IV_Freed");
				
				if (has_player_failed == true)
				force_rewind = false;
			}
		}
		else if (mail.message == "save and exit")
		{
			/*if (i_interface != nullptr)
			{
				delete i_interface;
				i_interface = nullptr;
			}*/

			//show_interface = false;
			game_pause = false;

			//保存的逻辑
			save();

			send(MessageQueue::game_code, "back to main page");
		}
		else if (mail.message == "save and close window")
		{
			if (i_interface != nullptr)
			{
				delete i_interface;
				i_interface = nullptr;
			}

			show_interface = false;
			game_pause = false;

			//保存的逻辑
			save();

			send(MessageQueue::game_code, "close window");
		}
	}
	//筛选出来自界面的消息
	else if (MessageQueue::computeInterclassicCode(mail.from) == MessageQueue::interface_header)
	{
		if (i_interface != nullptr)
		{
			delete i_interface;
			i_interface = nullptr;
		}

		if (mail.message == "reset game")
		{
			if (dialogue_interface != nullptr)
			{
				delete dialogue_interface;
				dialogue_interface = nullptr;
			}
			this->window->setMouseCursorVisible(false);

			show_interface = false;
			game_pause = false;
			endOfGame = false;
			has_player_failed = false;
			is_rewinding_time = false;

			spark_num = 0;
			points = 0;

			second = 0;
			minute = 0;
			fps_counter = 0;

			MessageQueue::resetPostalCodeCounter();

			setDialogueTrigger();

			if (player != nullptr)
				delete player;
			player = new Player(player_respwan_point.x, player_respwan_point.y, object_manager);
			map.spawnObject(object_manager, script_map.name, map_note, final, player);
			
			delete time_manager;
			time_manager = new TimeManagement;

			if (full_screen_quad != nullptr)
			{
				delete full_screen_quad;
				full_screen_quad = nullptr;
			}

			state = 0;
			begin_end_counter = MAP_BEGIN_AND_END_TIME;

			ResourceManagement::setBackgroundMusic(script_map.bgm);

			if (script_map.move_camera)
			{
				moving_camera = 1;
				move_camera_speed = 0.f;
				setViewToTheOtherSide();
			}
			else
			{
				//设置视角
				setViewToPlayer();
			}
		}
		else if (mail.message == "back to game")
		{
			if (i_interface != nullptr)
			{
				delete i_interface;
				i_interface = nullptr;
			}

			if (dialogue_interface == nullptr)
				this->window->setMouseCursorVisible(false);

			show_interface = false;
			game_pause = false;
			ResourceManagement::playBackgroundMusic();

			player->stop();
			//send(MessageQueue::player_code, "stop all motion");
		}
	}
	//筛选出来自玩家的消息
	else if (mail.from == MessageQueue::player_code)
	{
		if (mail.message == "camera shake")
		{
			camera_shake_counter = CAMERA_SHAKE_TIME;
		}
		else if (mail.message == "complete the game")
		{
			state = 2;
			begin_end_counter = MAP_BEGIN_AND_END_TIME;
		}
		else if (mail.message == "failed")
		{
			has_player_failed = true;
			is_rewinding_time = true;
			auto_rewind_time_counter = LONGEST_AUTO_REWIND_SECOND * FPS;
			ResourceManagement::stopSounds();
			ResourceManagement::setBackgroundMusic("II_Reverted");
			force_rewind = true;
		}
		else if (mail.message == "play bgm")
		{
			ResourceManagement::setBackgroundMusic(script_map.bgm);
		}
	}
	//筛选出来自对话界面的消息
	else if (MessageQueue::computeInterclassicCode(mail.from) == MessageQueue::dialogue_interface_header)
	{
		std::string prefix = "goto dialogue ";
		if (mail.message.substr(0, prefix.size()) == prefix) 
		{
			// 提取数字部分
			std::string numberString = mail.message.substr(prefix.size());
			// 将字符串转换为整数
			int dialogueId = std::stoi(numberString);

			//若对话没有结束，则进行下一段对话
			if (dialogueId != -1)
			{
				if (dialogue_interface->getIsNPCDialogues() == false)
				{
					if (dialogue_interface != nullptr)
						delete dialogue_interface;
					dialogue_interface = nullptr;
					dialogue_interface = new DialogueInterface(Script::findDialogue(script_map, dialogueId), false);
				}
				else
				{
					if (dialogue_interface != nullptr)
						delete dialogue_interface;
					dialogue_interface = nullptr;
					//查找NPC
					for (const auto& npc : map_note.npcs)
					{
						if (npc.npc_id == npc_info.current_speaking_npc_id)
						{
							for (const auto& dialogue : npc.dialogues)
							{
								//查找对话
								if (dialogue.dialogue_id == dialogueId)
								{
									dialogue_interface = new DialogueInterface(dialogue, true);
									break;
								}
							}
							break;
						}
					}
				}
			}
			else
			{
				if (dialogue_interface->getIsNPCDialogues())
				{
					send(npc_info.current_speaking_npc_postal_code, "dialogue end");
					this->npc_info.current_speaking_npc_id = 0;
					this->npc_info.current_speaking_npc_postal_code = MessageQueue::blank_code;
				}

				if (dialogue_interface != nullptr)
					delete dialogue_interface;
				dialogue_interface = nullptr;
				//退出剧情模式
				send(MessageQueue::player_code, "dialogue mode end");
				//隐藏鼠标
				this->window->setMouseCursorVisible(false);
				//剧情相当于存档点，无法倒流到剧情之前
				this->time_manager->clear();
			}
		}
	}
	//筛选出来自NPC的消息
	else if (MessageQueue::computeInterclassicCode(mail.from) == MessageQueue::NPC_header)
	{
		std::string prefix = "dialogue begin, npc id ";
		if (mail.message == "end npc dialogue")
		{
			if (dialogue_interface != nullptr)
			{
				delete dialogue_interface;
				dialogue_interface = nullptr;
				//退出剧情模式
				send(MessageQueue::player_code, "dialogue mode end");
				//隐藏鼠标
				this->window->setMouseCursorVisible(false);
			}
		}
		else if (mail.message.substr(0, prefix.size()) == prefix)
		{
			// 提取数字部分
			std::string numberString = mail.message.substr(prefix.size());
			// 将字符串转换为整数
			int npc_id = std::stoi(numberString);
			
			//查找NPC
			for (const auto& npc : map_note.npcs)
			{
				if (npc.npc_id == npc_id)
				{
					npc_info.current_speaking_npc_id = npc_id;
					npc_info.current_speaking_npc_postal_code = mail.from;
					//开始对话
					if (dialogue_interface == nullptr)
					{
						//进入剧情模式
						send(MessageQueue::player_code, "dialogue mode begin");
						if (is_rewinding_time == false)
							dialogue_interface = new DialogueInterface(npc.dialogues.at(0), true);
						else
							dialogue_interface = new DialogueInterface(npc.dialogues.at(npc.dialogue_id_while_rewinding_time - 1), true);
					}
					break;
				}
			}
		}
	}
	else if (MessageQueue::computeInterclassicCode(mail.from) == MessageQueue::receptor_header)
	{
		if (mail.message == "helmet on")
		{
			state = 2;
			begin_end_counter = MAP_BEGIN_AND_END_TIME;
		}
	}
}

void Scene::setDialogueTrigger()
{
	dialogue_trigger_zones.clear();
	for (const auto& dialogue_trigger : script_map.dialogue_triggers)
	{
		sf::FloatRect dialogue_trigger_zone;
		dialogue_trigger_zone.left = dialogue_trigger.trigger_zone.left;
		dialogue_trigger_zone.top = dialogue_trigger.trigger_zone.top;
		dialogue_trigger_zone.width = static_cast<float>(dialogue_trigger.trigger_zone.right - dialogue_trigger.trigger_zone.left);
		dialogue_trigger_zone.height = static_cast<float>(dialogue_trigger.trigger_zone.bottom - dialogue_trigger.trigger_zone.top);
		
		DialogueTriggerZoneAndTriggerID dialogue_trigger_zone_and_trigger_id =
		{
			dialogue_trigger_zone,
			dialogue_trigger.trigger_dialogue_id,
			dialogue_trigger.trigger_id
		};
		dialogue_trigger_zones.push_back(dialogue_trigger_zone_and_trigger_id);
	}
}

void Scene::moveBackground()
{
	if (background == nullptr || script_map.global_mode)
		return;

	//移动背景 
	sf::Vector2f position = {
		(static_cast<float>(map_width_brick * CELL_SIZE) - static_cast<float>(background_texture->getSize().x))
		* (view.getCenter().x - static_cast<float>((VIEW_TRACKING_EDGE)*CELL_SIZE))
		/ static_cast<float>((map_width_brick - (VIEW_TRACKING_EDGE) * 2) * CELL_SIZE),
		(static_cast<float>(map_height_brick * CELL_SIZE) - static_cast<float>(background_texture->getSize().y))
		* (view.getCenter().y - static_cast<float>((VIEW_TRACKING_EDGE_VERTICAL)*CELL_SIZE))
		/ static_cast<float>((map_height_brick - (VIEW_TRACKING_EDGE_VERTICAL) * 2) * CELL_SIZE)
	};
	background->setPosition(position);
}

void Scene::trackPlayer()
{
	if (script_map.global_mode)
		return;

	sf::Vector2f view_center = view.getCenter();
	sf::Vector2f target = this->player->getCoord();
	if (earthquake || camera_shake_counter > 0U)
	{
		if (play_quake_music == false)
		{
			play_quake_music = true;
			ResourceManagement::setBackgroundMusic("IV_Freed");
		}

		if (random_quake_counter == 0)
		{
			std::uniform_real_distribution<> random_x(MIN_AMPLITUDE * 16, MAX_AMPLITUDE * 16);
			std::uniform_real_distribution<> random_y(MIN_AMPLITUDE * 9, MAX_AMPLITUDE * 9);
			
			target.x += (shake_direction ? 1 : -1) * random_x(gen);
			target.y += (shake_direction ? 1 : -1) * random_y(gen);
			shake_direction = !shake_direction;
		}
		random_quake_counter++;
		if (random_quake_counter >= RANDOM_QUAKE_COUNTER)
			random_quake_counter = 0U;
		if (camera_shake_counter > 0U)
			camera_shake_counter--;
	}

	if (!earthquake && play_quake_music)
	{
		ResourceManagement::setBackgroundMusic(script_map.bgm);
		play_quake_music = false;
	}

	sf::Vector2f new_center;
	if (target.x < VIEW_TRACKING_EDGE * CELL_SIZE)
	{
		target.x = VIEW_TRACKING_EDGE * CELL_SIZE;
	}
	else if (target.x > (map.getMapWidthBlocks() - VIEW_TRACKING_EDGE) * CELL_SIZE)
	{
		target.x = (map.getMapWidthBlocks() - VIEW_TRACKING_EDGE) * CELL_SIZE;
	}

	if (target.y < VIEW_TRACKING_EDGE_VERTICAL * CELL_SIZE)
	{
		target.y = VIEW_TRACKING_EDGE_VERTICAL * CELL_SIZE;
	}
	else if (target.y > (map.getMapHeigthBlocks() - VIEW_TRACKING_EDGE_VERTICAL) * CELL_SIZE)
	{
		target.y = (map.getMapHeigthBlocks() - VIEW_TRACKING_EDGE_VERTICAL) * CELL_SIZE;
	}

	new_center.x = view_center.x + (target.x - view_center.x) * TRACK_SPEED;

	new_center.y = view_center.y + (target.y + CELL_SIZE / 2 - view_center.y) * TRACK_SPEED;	//向下偏移一点
	this->setView(new_center);

	moveBackground();
}

void Scene::setViewToPlayer()
{
	if (script_map.global_mode)
		return;

	sf::Vector2f view_center = view.getCenter();
	sf::Vector2f target = this->player->getCoord();
	sf::Vector2f new_center;
	if (target.x < VIEW_TRACKING_EDGE * CELL_SIZE)
	{
		target.x = VIEW_TRACKING_EDGE * CELL_SIZE;
	}
	else if (target.x > (map.getMapWidthBlocks() - VIEW_TRACKING_EDGE) * CELL_SIZE)
	{
		target.x = (map.getMapWidthBlocks() - VIEW_TRACKING_EDGE) * CELL_SIZE;
	}

	if (target.y < VIEW_TRACKING_EDGE_VERTICAL * CELL_SIZE)
	{
		target.y = VIEW_TRACKING_EDGE_VERTICAL * CELL_SIZE;
	}
	else if (target.y > (map.getMapHeigthBlocks() - VIEW_TRACKING_EDGE_VERTICAL) * CELL_SIZE)
	{
		target.y = (map.getMapHeigthBlocks() - VIEW_TRACKING_EDGE_VERTICAL) * CELL_SIZE;
	}

	new_center.x = target.x;

	new_center.y = target.y + CELL_SIZE / 2;	//向下偏移一点

	this->setView(new_center);

	moveBackground();
}

void Scene::setViewToTheOtherSide()
{
	if (script_map.global_mode)
		return;

	sf::Vector2f view_center = view.getCenter();
	sf::Vector2f target = this->player->getCoord();
	sf::Vector2f new_center;
	target.x = (map.getMapWidthBlocks() - VIEW_TRACKING_EDGE) * CELL_SIZE;
	if (target.y < VIEW_TRACKING_EDGE_VERTICAL * CELL_SIZE)
	{
		target.y = VIEW_TRACKING_EDGE_VERTICAL * CELL_SIZE;
	}
	else if (target.y > (map.getMapHeigthBlocks() - VIEW_TRACKING_EDGE_VERTICAL) * CELL_SIZE)
	{
		target.y = (map.getMapHeigthBlocks() - VIEW_TRACKING_EDGE_VERTICAL) * CELL_SIZE;
	}

	new_center.x = target.x;

	new_center.y = target.y + CELL_SIZE / 2;	//向下偏移一点

	this->setView(new_center);

	moveBackground();
}

void Scene::setGlobalMode()
{
	if (map_width_brick / map_height_brick >= static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT)
	{
		view.setSize({
			static_cast<float>(map_width_brick) * static_cast<float>(CELL_SIZE),
			map_width_brick * static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT * CELL_SIZE
			});
		//view.zoom(static_cast<float>(SCREEN_WIDTH) / (static_cast<float>(map_width_brick) * static_cast<float>(CELL_SIZE)));
		if (background != nullptr)
			background->setScale(view.getSize().x / background->getGlobalBounds().width, view.getSize().x / background->getGlobalBounds().width);
	}
	else
	{
		view.setSize({
			map_height_brick * static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT * CELL_SIZE,
			static_cast<float>(map_height_brick) * static_cast<float>(CELL_SIZE)
			});
		//view.zoom(static_cast<float>(SCREEN_WIDTH) / (map_height_brick * static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT * CELL_SIZE));
		if (background != nullptr)
			background->setScale(view.getSize().y / background->getGlobalBounds().height, view.getSize().y / background->getGlobalBounds().height);
	}
	view.setCenter(view.getSize().x / 2, view.getSize().y / 2);
	this->window->setView(view);

	render_texture.create(view.getSize().x * 4, view.getSize().y * 4);
	screen_sprite.setScale(0.25, 0.25);
	render_texture.setView(view);

	screen_sprite.setPosition(0, 0);

	if (background != nullptr)
		background->setPosition(view.getCenter().x - background->getGlobalBounds().width / 2, view.getCenter().y - background->getGlobalBounds().height / 2);
}

Scene::Scene(Script::Map& i_script_map, sf::RenderWindow* i_render_window)
{
	MessageQueue::resetPostalCodeCounter();

	script_map = i_script_map;

	this->initVar();
	this->initWin(i_render_window);
	this->initObject(script_map.name);

	map_width_brick = map.getMapWidthBlocks();
	map_height_brick = map.getMapHeigthBlocks();

	//设置背景
	if (script_map.background != "")
	{
		background = new sf::Sprite;
		background_texture = new sf::Texture;
		auto file_dir = EXE_DIR / "Resources" / "Background" / (script_map.background + ".png");
		background_texture->loadFromFile(file_dir.string());
		background->setTexture(*background_texture);
		if (script_map.global_mode)
			setGlobalMode();
	}

	if (script_map.move_camera == true)
		moving_camera = 1;

	if (script_map.recorded_timeline_filename != "")
		loadRecord();
}

Scene::Scene(Script::Map& i_script_map, sf::RenderWindow* i_render_window, bool i_load)
{
	MessageQueue::resetPostalCodeCounter();

	script_map = i_script_map;

	this->initVar();
	this->initWin(i_render_window);

	if (i_load == false)
	{
		this->initObject(script_map.name);

		if (script_map.recorded_timeline_filename != "")
			loadRecord();
	}
	else
	{
		this->object_manager = new ObjectManagement;

		map.loadFromFile(script_map.name, object_manager);
		map_width_brick = map.getMapWidthBlocks();
		map_height_brick = map.getMapHeigthBlocks();

		load();

		if (script_map.description != L"")
		{
			this->text = new sf::Text;	//展示地图描述
			this->text->setFont(*ResourceManagement::getFont(0));
			this->text->setString(script_map.description);
			this->text->setCharacterSize(30 * 4);
			this->text->setScale(0.25, 0.25);
			this->text->setFillColor(sf::Color::White);
			this->text->setPosition(SCREEN_WIDTH / 2 - this->text->getGlobalBounds().width / 2, SCREEN_HEIGHT / 2 - this->text->getGlobalBounds().height / 2 - 6);
		}
		else
		{
			ResourceManagement::setBackgroundMusic(script_map.bgm);
			setViewToPlayer();
		}

		this->time_manager = new TimeManagement(true);

		this->full_screen_quad = nullptr;

		this->bar = new Bar;

		this->i_interface = nullptr;
		this->dialogue_interface = nullptr;

		this->has_player_failed = player->getHasFailed();

		
	}

	//设置背景
	if (script_map.background != "")
	{
		background = new sf::Sprite;
		background_texture = new sf::Texture;
		auto file_dir = EXE_DIR / "Resources" / "Background" / (script_map.background + ".png");
		background_texture->loadFromFile(file_dir.string());
		background->setTexture(*background_texture);
	}

	moving_camera = false;
	if (script_map.global_mode && script_map.description == L"" && i_load == true)
		setGlobalMode();
}

Scene::~Scene()
{
	delete this->player;
	/*delete this->view;*/
	delete this->bar;
	delete this->object_manager;
	delete this->time_manager;
	if (full_screen_quad != nullptr)
		delete full_screen_quad;
	if (background != nullptr)
		delete background;
	if (background_texture != nullptr)
		delete background_texture;
	if (i_interface != nullptr)
		delete i_interface;
	if (dialogue_interface != nullptr)
		delete dialogue_interface;
	if (press_f_reminder != nullptr)
		delete press_f_reminder;
	if (cheating_mode_reminder != nullptr)
		delete cheating_mode_reminder;

#if SHOW_CONSOLE
	if (cheating_mode_reminder == nullptr)
		delete cheating_mode_reminder;
#endif

	ResourceManagement::stopBackgroundMusic();

	if (script_map.global_mode)
		view.setSize({ SCREEN_WIDTH, SCREEN_HEIGHT });
}

void Scene::update()
{
	checkAllMail();

	if (text != nullptr)
	{
		begin_end_counter--;
		if (begin_end_counter == 0)
		{
			delete text;
			text = nullptr;
			begin_end_counter = MAP_BEGIN_AND_END_TIME;
			//播放背景音乐
			ResourceManagement::setBackgroundMusic(script_map.bgm);
			//设置视角
			if (script_map.global_mode)
				setGlobalMode();
			else
			{
				if (script_map.move_camera == false)
					setViewToPlayer();
				else
					setViewToTheOtherSide();
			}
		}
		else
			return;
	}

	if (!game_pause)
	{
		//视角跟随玩家
		if (!endOfGame && moving_camera != 1)
		{
			earthquake = false;
			for (const auto& quake_zone : script_map.quake_zone)
			{
				if (MapCollision::object_collision(player->getHitBox(), &quake_zone))
					earthquake = true;
			}

			this->trackPlayer();
		}
		else if (moving_camera == 1)
		{
			if (abs(player->getCoord().x - view.getCenter().x) > SCREEN_WIDTH / 3 && view.getCenter().x >= SCREEN_WIDTH / 2)
			{
				if (move_camera_speed < 4.f)
					move_camera_speed += 0.03f;
				view.move({ -move_camera_speed, 0 });
				this->window->setView(view);
				moveBackground();
			}
			else
			{
				moving_camera = 2;
			}
		}

		if (begin_end_counter != 0 && state == 0)
		{
			begin_end_counter--;
			if (full_screen_quad == nullptr)
			{
				full_screen_quad = new sf::RectangleShape;
				this->full_screen_quad->setSize({ view.getSize().x, view.getSize().y });
				this->full_screen_quad->setPosition(view.getCenter().x - view.getSize().x / 2,
					view.getCenter().y - view.getSize().y / 2);
				this->full_screen_quad->setFillColor(sf::Color::Black);
			}
			else
			{
				this->full_screen_quad->setPosition(view.getCenter().x - view.getSize().x / 2,
					view.getCenter().y - view.getSize().y / 2);
				this->full_screen_quad->setFillColor(sf::Color(0, 0, 0,
					255.f * static_cast<float>(begin_end_counter) / static_cast<float>(MAP_BEGIN_AND_END_TIME)));
			}
			if (begin_end_counter == 0)
			{
				delete this->full_screen_quad;
				this->full_screen_quad = nullptr;
				state = 1;
			}
		}
		else if (begin_end_counter != 0 && state >= 2)
		{
			begin_end_counter--;
			if (state == 2)
			{
				if (full_screen_quad != nullptr)
				{
					delete full_screen_quad;
					full_screen_quad = nullptr;
				}

				full_screen_quad = new sf::RectangleShape;
				this->full_screen_quad->setSize({ view.getSize().x, view.getSize().y });
				this->full_screen_quad->setPosition(view.getCenter().x - view.getSize().x / 2,
					view.getCenter().y - view.getSize().y / 2);
				this->full_screen_quad->setFillColor(sf::Color(0, 0, 0, 0));

				state++;	//防止重复创建
			}
			else
			{
				this->full_screen_quad->setPosition(view.getCenter().x - view.getSize().x / 2,
					view.getCenter().y - view.getSize().y / 2);
				this->full_screen_quad->setFillColor(sf::Color(0, 0, 0,
					std::min(255.f, 255.f * (1 - (static_cast<float>(begin_end_counter) / static_cast<float>(MAP_BEGIN_AND_END_TIME))))));
				ResourceManagement::setBackgroundMusicVolume(ResourceManagement::getBackgroundMusicVolume() * 0.95);
			}
			if (begin_end_counter == 0)
			{
				endOfGame = true;
				send(MessageQueue::game_code, "game completed");
			}
		}

		if (this->show_interface == false && this->i_interface != nullptr)
		{
			delete i_interface;
			i_interface = nullptr;
		}

		if (this->endOfGame == false)
		{
			object_manager->setLoadingRange(view.getCenter());

			if (is_rewinding_time)
			{
				//计时
				if (!(minute == '\0' && second == '\0'))
				{
					if (fps_counter != 0)
					{
						fps_counter--;
					}
					else
					{
						fps_counter = FPS;
						if (second != 0)
						{
							second--;
						}
						else
						{
							second = 59;
							minute--;
						}
					}
				}

				//时间倒流
				loading_shader = true;
				time_manager->rewind(player, object_manager);
				object_manager->updateThoseWhoCouldntBeAffectedByTimeControl();

				if (has_player_failed && force_rewind)
				{
					if (object_manager->checkIsPlayerSafe() || time_manager->getSize() <= 0 || auto_rewind_time_counter == 0)
					{
						has_player_failed = false;
						is_rewinding_time = false;
						if (!earthquake)
							ResourceManagement::setBackgroundMusic(script_map.bgm);
						else
							ResourceManagement::setBackgroundMusic("IV_Freed");
					}
					auto_rewind_time_counter--;
				}

			}
			else
			{
				//计时
				fps_counter++;
				if (fps_counter == FPS)
				{
					fps_counter = 0;
					second++;
					if (second == 60)
					{
						second = 0;
						minute++;
					}
				}
				if (second == script_map.time_limit.second && minute == script_map.time_limit.minute)
				{
					if (dialogue_interface != nullptr)
						delete dialogue_interface;
					Script::Dialogue time_limit_info = {
						0,
						L"",
							script_map.time_limit.message
					};
					time_limit_info.effect.push_back(Script::Expression({ "reset_game", "", 0 }));
					dialogue_interface = new DialogueInterface(time_limit_info, false);
					player->stop();
				}

				
				//时间正常流动
				loading_shader = false;

				//检测是否达到重点
				if (MapCollision::object_collision(player->getHitBox(), &final) && state < 2)
				{
					state = 2;
					begin_end_counter = MAP_BEGIN_AND_END_TIME;
				}

				//检查是否触发剧情
				for (auto dialogue_trigger = dialogue_trigger_zones.begin(); dialogue_trigger != dialogue_trigger_zones.end(); dialogue_trigger++)
				{
					if (MapCollision::object_collision(&(*dialogue_trigger).dialogue_trigger_zone, player->getHitBox()) && dialogue_interface == nullptr)
					{
						//启动对话界面
						dialogue_interface = new DialogueInterface(Script::findDialogue(script_map, (*dialogue_trigger).trigger_dialouge_id), false);
						//防止再次触发
						dialogue_trigger_zones.erase(dialogue_trigger);
						//进入剧情模式
						send(MessageQueue::player_code, "dialogue mode begin");
						//显示鼠标
						this->window->setMouseCursorVisible(true);
						break;
					}
				}

				this->object_manager->update();

				time_manager->record(player, object_manager);
			}

		}

		if (object_manager->getCanInteract())
		{
			if (press_f_reminder == nullptr)
			{
				press_f_reminder = new Text("F");
				press_f_reminder->setFont(0);
				press_f_reminder->setFillColor(sf::Color::White);
				press_f_reminder->setCharacterSize(18);
			}
			press_f_reminder->setPosition(player->getCoord().x + CELL_SIZE * 1.5, player->getCoord().y - CELL_SIZE * 1.5);
		}
		else if (!object_manager->getCanInteract() && press_f_reminder != nullptr)
		{
			delete press_f_reminder;
			press_f_reminder = nullptr;
		}
		
		if (bar != nullptr)
		{
			this->bar->updateViewCenter(view.getCenter());
			bar->update();
		}

		if (dialogue_interface != nullptr)
		{
			dialogue_interface->updateViewCenter(view.getCenter());
			dialogue_interface->update(window);
		}
	}

	if (i_interface != nullptr)
	{
		i_interface->updateViewCenter(view.getCenter());
		i_interface->update(window);
	}

#if SHOW_CONSOLE
	if (cheating_mode)
	{
		if (cheating_mode_reminder == nullptr)
		{
			cheating_mode_reminder = new Text("CHEATING MODE ON");
			cheating_mode_reminder->setCharacterSize(20);
		}
		cheating_mode_reminder->setPosition(view.getCenter().x + SCREEN_WIDTH / 2 - 190, view.getCenter().y - SCREEN_HEIGHT / 2 + 10);
	}
	if (!cheating_mode && cheating_mode_reminder != nullptr)
	{
		delete cheating_mode_reminder;
		cheating_mode_reminder = nullptr;
	}
#endif
}

void Scene::render()
{
	float scale_factor = view.getSize().x / SCREEN_WIDTH;

	render_texture.setView(view);
	render_texture.clear(sf::Color::Transparent);
	if (script_map.global_mode == false)
	{
		screen_sprite.setPosition(
			view.getCenter().x - SCREEN_WIDTH * 0.5,
			view.getCenter().y - SCREEN_HEIGHT * 0.5
		);
	}

	if (shader_available)
	{
		auto time = timer.getEscapeTime() / 1000;
		if (is_rewinding_time)
		{
			shader.setUniform("time", time);
			shader.setUniform("useShader", true);
		}
		else
			shader.setUniform("useShader", false);
		shader2.setUniform("time", time);
	}

	if (background != nullptr)
	{
		this->render_texture.draw(*background);
	}

	if (text != nullptr)
	{
		this->window->clear(sf::Color::Black);
		this->window->draw(*text);
		return;
	}

	//map.setRenderingRange(view.getCenter());
	this->render_texture.draw(map);

	object_manager->draw(render_texture);
	/*sf::RectangleShape s;
	s.setFillColor(sf::Color::Transparent);
	s.setOutlineColor(sf::Color::Blue);
	s.setOutlineThickness(1);
	s.setSize({CELL_SIZE, CELL_SIZE});
	s.setPosition(player->getCoord().x - 8, player->getCoord().y - 8);
	window->draw(s);*/

	if (press_f_reminder != nullptr)
		this->render_texture.draw(*press_f_reminder);

	/*sf::RectangleShape r;
	r.setPosition(view->getCenter().x - 3, view->getCenter().y - 3);
	r.setSize({ 5.9, 5.9 });
	r.setFillColor(sf::Color::Red);
	render_texture.draw(r);*/

	render_texture.display();
	screen_sprite.setTexture(render_texture.getTexture());
	window->draw(screen_sprite, states);

	object_manager->draw(*window, states2);

	if (dialogue_interface != nullptr)
	{
		dialogue_interface->setScale({ scale_factor, scale_factor });
		this->window->draw(*dialogue_interface);
	}

	this->window->draw(*bar);

#if DRAW_ZONE
	for (auto& temp : dialogue_trigger_zones)
	{
		sf::RectangleShape t;
		t.setFillColor(sf::Color(255, 0, 255, 70));
		t.setOutlineColor(sf::Color::Magenta);
		t.setOutlineThickness(1);
		t.setPosition(temp.dialogue_trigger_zone.left, temp.dialogue_trigger_zone.top);
		t.setSize({ temp.dialogue_trigger_zone.width, temp.dialogue_trigger_zone.height });
		this->window->draw(t);
	}

	sf::RectangleShape t;
	t.setFillColor(sf::Color(255, 255, 0, 70));
	t.setOutlineColor(sf::Color::Yellow);
	t.setOutlineThickness(1);
	t.setPosition(final.left, final.top);
	t.setSize({ final.width, final.height });
	this->window->draw(t);

	for (const auto& quake_zone : script_map.quake_zone)
	{
		sf::RectangleShape q;
		q.setFillColor(sf::Color(0, 255, 255, 70));
		q.setOutlineColor(sf::Color::Cyan);
		q.setOutlineThickness(1);
		q.setPosition(quake_zone.left, quake_zone.top);
		q.setSize({ quake_zone.width, quake_zone.height });
		this->window->draw(q);
	}

#endif

	//设置滤镜
	if (loading_shader && state == 1 && shader_available == false)
	{
		if (full_screen_quad == nullptr)
			this->full_screen_quad = new sf::RectangleShape(sf::Vector2f(view.getSize().x, view.getSize().y));

		sf::Vector2f full_screen_quad_position =
		{
			view.getCenter().x - view.getSize().x / 2,
			view.getCenter().y - view.getSize().y / 2
		};

		this->full_screen_quad->setPosition(full_screen_quad_position);
		this->full_screen_quad->setFillColor(sf::Color(170, 7, 107, 100));

		this->window->draw(*full_screen_quad);
	}
	else
	{
		if (full_screen_quad != nullptr && state == 1)
		{
			delete full_screen_quad;
			full_screen_quad = nullptr;
		}
	}

	if (full_screen_quad != nullptr && (state == 0 || state >= 2))
	{
		this->window->draw(*full_screen_quad);
	}

	if (i_interface != nullptr)
	{
		i_interface->setScale({ scale_factor, scale_factor });
		this->window->draw(*i_interface);
	}

	if (VIEW_SIZE > 1)
	{
		sf::RectangleShape t;
		t.setPosition(view.getCenter().x - SCREEN_WIDTH * LOADING_FACTOR, view.getCenter().y - SCREEN_HEIGHT * LOADING_FACTOR);
		t.setSize({ SCREEN_WIDTH * 2 * LOADING_FACTOR, SCREEN_HEIGHT * 2 * LOADING_FACTOR });
		t.setFillColor(sf::Color::Transparent);
		t.setOutlineColor(sf::Color::Green);
		t.setOutlineThickness(3);
		window->draw(t);
	}

#if SHOW_CONSOLE
	if (cheating_mode_reminder != nullptr)
	{
		//cheating_mode_reminder->setScale({ scale_factor, scale_factor });
		window->draw(*cheating_mode_reminder);
	}
#endif
}

const bool Scene::canControlPlayer() const
{
	return i_interface == nullptr && dialogue_interface == nullptr;
}

void Scene::save()
{
	auto file_dir = EXE_DIR / "History" / "save.bin";
	std::ofstream file(file_dir, std::ios::binary | std::ios::app);
	if (!file.is_open())
	{
		std::cerr << "failed to open save file" << std::endl;
		return;
	}

	file.write(reinterpret_cast<char*>(&ending), sizeof(ending));

	//写入终点信息
	file.write(reinterpret_cast<char*>(&final), sizeof(final));

	file.write(reinterpret_cast<char*>(&has_player_failed), sizeof(has_player_failed));

	file.write(reinterpret_cast<char*>(&fps_counter), sizeof(fps_counter));
	file.write(reinterpret_cast<char*>(&second), sizeof(second));
	file.write(reinterpret_cast<char*>(&minute), sizeof(minute));

	MessageQueue::savePostalCodeCounter(file);

	std::vector<short> trigger_ids;
	for (auto& trigger : dialogue_trigger_zones)
	{
		trigger_ids.push_back(trigger.trigger_id);
	}

	//写入向量的大小并写入向量
	size_t size = trigger_ids.size();
	file.write(reinterpret_cast<char*>(&size), sizeof(size));
	file.write(reinterpret_cast<char*>(trigger_ids.data()), size * sizeof(short));

	//记录所有实体的信息
	Snap::ObjectSnap object_snap;
	Snap::WorldSnap world_snap;

	object_snap.coord = player->getCoord();
	object_snap.horizontal_speed = player->getHorizontalSpeed();
	object_snap.vertical_speed = player->getVerticalSpeed();
	object_snap.postal_code = player->getPostalCode();
	object_snap.eigen_code = player->saveCode();
	world_snap.object.push_back(object_snap);
	//将玩家数据储存在第一位
	//接下来储存敌人信息和其他实体的信息

	for (auto& enemy : object_manager->getEnemyList())
	{
		object_snap.coord = enemy->getCoord();
		object_snap.horizontal_speed = enemy->getHorizontalSpeed();
		object_snap.vertical_speed = enemy->getVerticalSpeed();
		object_snap.postal_code = enemy->getPostalCode();
		object_snap.eigen_code = enemy->saveCode();
		world_snap.object.push_back(object_snap);
	}

	for (auto& object : object_manager->getObjectList())
	{
		object_snap.coord = object->getCoord();
		object_snap.horizontal_speed = object->getHorizontalSpeed();
		object_snap.vertical_speed = object->getVerticalSpeed();
		object_snap.postal_code = object->getPostalCode();
		object_snap.eigen_code = object->saveCode();
		world_snap.object.push_back(object_snap);
	}
	
	//写入向量的大小并写入向量
	size = world_snap.object.size();
	file.write(reinterpret_cast<char*>(&size), sizeof(size));
	
	for (auto& object_snap : world_snap.object) 
	{
		file.write(reinterpret_cast<char*>(&object_snap), sizeof(object_snap));
	}

	file.close();

	time_manager->asyncSaveAllToFile();
}

void Scene::load()
{
	auto file_dir = EXE_DIR / "History" / "save.bin";
	std::ifstream file(file_dir, std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "failed to open save file" << std::endl;
	}

	size_t size = sizeof(ending_has_achieved) + sizeof(spark_num) + sizeof(points) + sizeof(player_respwan_point) + sizeof(int) * 2;
	file.seekg(size, std::ios::beg);

	file.read(reinterpret_cast<char*>(&ending), sizeof(ending));
	file.read(reinterpret_cast<char*>(&final), sizeof(final));
	file.read(reinterpret_cast<char*>(&has_player_failed), sizeof(has_player_failed));
	file.read(reinterpret_cast<char*>(&fps_counter), sizeof(fps_counter));
	file.read(reinterpret_cast<char*>(&second), sizeof(second));
	file.read(reinterpret_cast<char*>(&minute), sizeof(minute));

	MessageQueue::loadPostalCodeCounter(file);

	//读取触发器id
	//获取向量大小
	file.read(reinterpret_cast<char*>(&size), sizeof(size));

	//读取向量
	dialogue_trigger_zones.clear();
	std::vector<short> trigger_ids;
	trigger_ids.resize(size);
	file.read(reinterpret_cast<char*>(trigger_ids.data()), size * sizeof(short));

	for (auto& id : trigger_ids)
	{
		//file.read(reinterpret_cast<char*>(&id), sizeof(id));
		//根据id设置触发器
		for (const auto& dialogue_trigger : script_map.dialogue_triggers)
		{
			if (dialogue_trigger.trigger_id != id)
				continue;
			sf::FloatRect dialogue_trigger_zone;
			dialogue_trigger_zone.left = dialogue_trigger.trigger_zone.left;
			dialogue_trigger_zone.top = dialogue_trigger.trigger_zone.top;
			dialogue_trigger_zone.width = static_cast<float>(dialogue_trigger.trigger_zone.right - dialogue_trigger.trigger_zone.left);
			dialogue_trigger_zone.height = static_cast<float>(dialogue_trigger.trigger_zone.bottom - dialogue_trigger.trigger_zone.top);

			DialogueTriggerZoneAndTriggerID dialogue_trigger_zone_and_trigger_id =
			{
				dialogue_trigger_zone,
				dialogue_trigger.trigger_dialogue_id,
				dialogue_trigger.trigger_id
			};
			dialogue_trigger_zones.push_back(dialogue_trigger_zone_and_trigger_id);
			break;
		}
	}
	
	//生成map note中的实体
	std::list<Receptor*> rs;
	//尝试读取地图注释
	std::string map_note_file_path = "Resources/Maps/" + script_map.name + ".json";
	if (std::filesystem::exists(map_note_file_path))
	{
		//读取地图注释
		MapNote::parseMapNote(map_note_file_path, map_note);

		//生成movable wall
		for (const auto& i_movable_wall : map_note.movable_walls)
		{
			MovableWall* temp = new MovableWall(i_movable_wall.x * CELL_SIZE, i_movable_wall.y * CELL_SIZE, i_movable_wall.speed, i_movable_wall.length, object_manager);
			temp->init(i_movable_wall.structure_direction, i_movable_wall.move_direction, i_movable_wall.move_distance, i_movable_wall.set_can_be_affected_by_time_control);
		}

		//生成NPC
		for (const auto& i_npc : map_note.npcs)
		{
			new NPC(i_npc, object_manager);
		}

		//生成Receptor
		for (const auto& receptor : map_note.receptors_and_effectors)
		{
			Receptor* r;
			if (receptor.receptor.type == MapNote::ReceptorType::drawbar)
			{
				r = new Drawbar(receptor, object_manager);
			}
			else if (receptor.receptor.type == MapNote::ReceptorType::pressure_plate)
			{
				r = new PressurePlate(receptor, object_manager);
			}
			rs.push_back(r);
		}
	}

	//获取向量大小
	file.read(reinterpret_cast<char*>(&size), sizeof(size));

	//读取向量
	Snap::WorldSnap world_snap;
	world_snap.object.resize(size);

	std::list<ShadowGenerator*> sgs;

	for (size_t i = 0; i < size; i++)
	{
		Snap::ObjectSnap& object_snap = world_snap.object[i];
		file.read(reinterpret_cast<char*>(&object_snap), sizeof(object_snap));

		//复现实体
		Object* o = nullptr;
		bool found_receptor = false;
		switch (MessageQueue::computeInterclassicCode(object_snap.postal_code))
		{
		case MessageQueue::robot_header:
			o = new Robot(object_snap.coord.x, object_snap.coord.y, object_manager, object_snap.postal_code);
			o->setState(object_snap.coord, object_snap.horizontal_speed, object_snap.vertical_speed);
			o->praseSaveCode(object_snap.eigen_code);
			continue;
		case MessageQueue::spark_header:
			new Spark(object_snap.coord.x - 32, object_snap.coord.y - 32, object_manager, object_snap.postal_code);
			continue;
		case MessageQueue::NPC_header:
			continue;
		case MessageQueue::receptor_header:
			//遍历object_list，查找对应的receptor
			for (auto& object : object_manager->getObjectList())
			{
				if (object->getPostalCode() == object_snap.postal_code)
				{
					object->decode(object_snap.eigen_code);
					found_receptor = true;
					break;
				}
			}
			if (found_receptor == false)
			{
				new Helmet(object_snap.coord.x + CELL_SIZE / 2, object_snap.coord.y - CELL_SIZE / 2, object_manager);
			}
			continue;
		case MessageQueue::movable_wall_header:
			//遍历object_list，查找对应的movable wall
			for (auto& object : object_manager->getObjectList())
			{
				if (object->getPostalCode() == object_snap.postal_code)
				{
					object->setState(object_snap.coord, object_snap.horizontal_speed, object_snap.vertical_speed);
					object->praseSaveCode(object_snap.eigen_code);
					break;
				}
			}
			continue;
		case MessageQueue::laser_header:
			o = new Laser(object_snap.coord.x, object_snap.coord.y, object_manager, Direction::right, object_snap.postal_code);
			o->setState(object_snap.coord, object_snap.horizontal_speed, object_snap.vertical_speed);
			o ->praseSaveCode(object_snap.eigen_code);
			continue;
		case MessageQueue::laser_gun_header:
			o = new LaserGun(object_snap.coord.x, object_snap.coord.y, object_manager, 0, object_snap.postal_code);
			o->setState(object_snap.coord, object_snap.horizontal_speed, object_snap.vertical_speed);
			o->praseSaveCode(object_snap.eigen_code);
			continue;
		case MessageQueue::robot_factory_header:
			o = new RobotFactory(object_snap.coord.x, object_snap.coord.y, object_manager, 0);
			o->setState(object_snap.coord, object_snap.horizontal_speed, object_snap.vertical_speed);
			o->praseSaveCode(object_snap.eigen_code);
			continue;
		case MessageQueue::shadow_generator_header:
			o = new ShadowGenerator(object_snap.coord.x, object_snap.coord.y, object_manager, true, object_snap.postal_code);
			o->praseSaveCode(object_snap.eigen_code);
			reinterpret_cast<ShadowGenerator*>(o)->registerListener(std::bind(&Player::getStates, player,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			sgs.push_back(reinterpret_cast<ShadowGenerator*>(o));
			continue;
		default:
			break;
		}
		
		/*if (object_snap.postal_code == MessageQueue::flag_code)
		{
			Flag* f = new Flag(object_snap.coord.x, object_snap.coord.y, object_manager);
			f->decode(object_snap.eigen_code);
			continue;
		}*/
		if (object_snap.postal_code == MessageQueue::player_code)
		{
			this->player = new Player(object_snap.coord.x, object_snap.coord.y, object_manager);
			this->player->setState(object_snap.coord, object_snap.horizontal_speed, object_snap.vertical_speed);
			this->player->decode(object_snap.eigen_code);
			continue;
		}
	}

	if (sgs.size() > 0 && rs.size() > 0)
	{
		for (const auto& g : sgs)
		{
			for (auto r : rs)
			{
				g->registerListener(std::bind(&Receptor::getStateByPostalCode, r, std::placeholders::_1));
			}
		}
	}
}

void Scene::loadRecord()
{
	auto record_path = EXE_DIR / "History" / script_map.recorded_timeline_filename;
	auto target_path = EXE_DIR / "History" / "timeline.bin";
	std::ifstream source(record_path, std::ios::binary);
	if (!source.is_open())
		throw std::logic_error("Open file " + record_path.string() + " failed");
	std::ofstream target(target_path, std::ios::binary);
	if (!target.is_open())
		throw std::logic_error("Open file " + target_path.string() + " failed");

	char buffer[1024];
	while (source.read(buffer, sizeof(buffer)))
		target.write(buffer, source.gcount());
	if (source.gcount() > 0)
		target.write(buffer, source.gcount());

	source.close();
	target.close();
}