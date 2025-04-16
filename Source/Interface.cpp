#include "Interface.h"

Interface::Interface()
{
	this->postal_code = MessageQueue::getPostalCode(MessageQueue::interface_header);
	current_button = 0;
	button_num = 0;
	choose_box = nullptr;
	delay = INTERFACE_KEYBOARD_SELECT_DELAY;
	begin_delay = INTERFACE_KEYBOARD_SELECT_DELAY;
}

Interface::~Interface()
{
	element_list.clear();
	for (auto text : text_list)
		delete text;
	text_list.clear();
	if (choose_box != nullptr)
		delete choose_box;
}

void Interface::updateViewCenter(sf::Vector2f view_center)
{
	position.x = view_center.x - view.getSize().x / 2;
	position.y = view_center.y - view.getSize().y / 2;
}

void Interface::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();

	for (int i = 0; i < text_list.size(); i++)
	{
		sf::Vector2f text_center =
		{
			position.x + element_list.at(i).center.x,
			position.y + element_list.at(i).center.y
		};
		text_list.at(i)->setCenter(text_center);
		target.draw(*text_list.at(i), states);

		/*auto bounds = text_list.at(i)->getGloabalBounds();
		sf::RectangleShape rect({bounds.width, bounds.height});
		rect.setPosition(bounds.left, bounds.top);
		rect.setFillColor(sf::Color(255, 255, 255, 100));
		target.draw(rect);*/
	}

	if (button_num > 0)
		target.draw(*choose_box, states);

}

void Interface::update(sf::RenderWindow* window)
{
	if (begin_delay > 0)
	{
		begin_delay--;
		return;
	}

	if (delay != 0)
	{
		delay--;
	}

	if (button_num > 0)
	{
		//??????
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && window->hasFocus() && delay == 0)
		{
			delay = INTERFACE_KEYBOARD_SELECT_DELAY;

			sf::Vector2i mousePosWindow = sf::Mouse::getPosition(*window);
			sf::Vector2f mousePosView = window->mapPixelToCoords(mousePosWindow);
			float scale_factor = view.getSize().x / SCREEN_WIDTH;
			Text* text;
			sf::FloatRect text_bounds;

			for (int i = 0; i < text_list.size(); i++)
			{
				if (element_list.at(i).can_be_press == false)
					continue;

				text = text_list.at(i);
				text_bounds = text->getGloabalBounds();
				text_bounds.left *= scale_factor;
				text_bounds.top *= scale_factor;
				text_bounds.width *= scale_factor;
				text_bounds.height *= scale_factor;

				if (text_bounds.contains(mousePosView))
				{
					element_list.at(i).function();
				}
			}
		}

		setChooseBox();

	}

	checkMailBoxAndReact();
}

void Interface::init()
{
	int max_width = 0;

	for (auto& element : element_list)
	{
		if (element.can_be_press)	//???????????
			button_num++;

		Text* text = new Text;
		text->setCharacterSize(element.character_size);
		text->setFont(element.font_id);
		text->setMessage(element.message);

		sf::Vector2f text_center =
		{
			position.x + element.center.x,
			position.y + element.center.y
		};
		text->setCenter(text_center);

		if (element.align)
		{
			if (text->getBackgroundWidth() > max_width)
			{
				max_width = text->getBackgroundWidth();
			}
		}

		text_list.push_back(text);
	}
	for (int i = 0; i < element_list.size(); i++)
	{
		if (element_list.at(i).align)
			text_list.at(i)->setBackgroundWidthForcely(max_width + CELL_SIZE * 4);
	}

	//????????
	if (button_num > 0)
	{
		choose_box = new sf::RectangleShape();
		choose_box->setFillColor(sf::Color(0, 0, 0, 0));
		choose_box->setOutlineColor(sf::Color::White);
		choose_box->setOutlineThickness(1);
	}
}

void Interface::react()
{
	MessageQueue::Mail mail = check();
	if (mail.from == MessageQueue::blank_code)
		return;

	if (mail.message == "up is pressed" || mail.message == "left is pressed")
	{
		if (delay == 0)
		{
			if (current_button > 0)
				current_button--;
			else
				current_button = button_num - 1;
			delay = INTERFACE_KEYBOARD_SELECT_DELAY;
		}
	}
	else if (mail.message == "down is pressed" || mail.message == "right is pressed")
	{
		if (delay == 0)
		{
			if (current_button < button_num - 1)
				current_button++;
			else
				current_button = 0;
			delay = INTERFACE_KEYBOARD_SELECT_DELAY;
		}
	}
	else if (mail.message == "enter is pressed")
	{
		if (delay == 0)
		{
			for (int i = 0; i < element_list.size(); i++)
			{
				if (element_list.at(i).can_be_press)
				{
					if (current_button == 0)
					{
						element_list.at(i).function();
						return;
					}
					else
					{
						current_button--;
					}
				}
				delay = INTERFACE_KEYBOARD_SELECT_DELAY;
			}
		}
	}
}

void Interface::setChooseBox()
{
	if (button_num == 0)
		return;

	unsigned short counter = 0;
	for (int i = 0; i < element_list.size(); i++)
	{
		if (element_list.at(i).can_be_press)
		{
			if (counter == current_button)
			{
				sf::Vector2f text_center =
				{
					position.x + element_list.at(i).center.x,
					position.y + element_list.at(i).center.y
				};
				text_list.at(i)->setCenter(text_center);
				sf::FloatRect bounds = text_list.at(i)->getGloabalBounds();
				choose_box->setPosition(bounds.left, bounds.top);
				choose_box->setSize({ bounds.width, bounds.height });
				return;
			}
			counter++;
		}
	}
}

PauseInterface::PauseInterface()
{
	const auto& view_width = view.getSize().x;
	const auto& view_height = view.getSize().y;
	//float scale_factor = view.getSize().x / SCREEN_WIDTH;

	InterfaceElement info =
	{
		L"游戏暂停中",
		sf::Vector2f({ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 3 - 5}),
		false,
		25,
		0,
		nullptr
	};

	InterfaceElement back_button =
	{
		L"回到游戏",
		sf::Vector2f({ SCREEN_WIDTH / 2, 4 * SCREEN_HEIGHT / 9}),
		true,
		static_cast<unsigned char>(DEFAULT_CHARACTER_SIZE),
		0,
		std::bind(&PauseInterface::backToGame, this)
	};

	InterfaceElement reset_button =
	{
		L"重置本局",
		sf::Vector2f({SCREEN_WIDTH / 2, 5 * SCREEN_HEIGHT / 9}),
		true,
		static_cast<unsigned char>(DEFAULT_CHARACTER_SIZE),
		0,
		std::bind(&PauseInterface::reset, this)  // ???std::bind????????
	};
	InterfaceElement end_button =
	{
		L"保存并回到主界面",
		sf::Vector2f({ SCREEN_WIDTH / 2, 6 * SCREEN_HEIGHT / 9}),
		true,
		static_cast<unsigned char>(DEFAULT_CHARACTER_SIZE),
		0,
		std::bind(&PauseInterface::saveAndExit, this)
	};

	element_list.push_back(info);
	element_list.push_back(back_button);
	element_list.push_back(reset_button);
	element_list.push_back(end_button);
	init();
}

void PauseInterface::reset()
{
	send(MessageQueue::scene_code, "reset game");
}

void PauseInterface::backToGame()
{
	send(MessageQueue::scene_code, "back to game");
}

void PauseInterface::saveAndExit()
{
	//game??????scene?????
	send(MessageQueue::game_code, "save and exit");
}

StartInterface::StartInterface()
{
	//???浵??????
	auto file_dir = EXE_DIR / "History" / "save.bin";
	if (std::filesystem::exists(file_dir) == false)
	{
		InterfaceElement new_game_button =
		{
			L"新游戏",
			sf::Vector2f({SCREEN_WIDTH / 2, 0.7 * SCREEN_HEIGHT}),
			true,
			static_cast<unsigned char>(DEFAULT_CHARACTER_SIZE),
			0,
			std::bind(&StartInterface::newGame, this)
		};

		InterfaceElement quit_button =
		{
			L"退出游戏",
			sf::Vector2f({SCREEN_WIDTH / 2, 0.8 * SCREEN_HEIGHT}),
			true,
			static_cast<unsigned char>(DEFAULT_CHARACTER_SIZE),
			0,
			std::bind(&StartInterface::quit, this)
		};

		element_list.push_back(new_game_button);
		element_list.push_back(quit_button);
	}
	else
	{
		InterfaceElement continue_game_button =
		{
			L"继续游戏",
			sf::Vector2f({SCREEN_WIDTH / 2, 0.6 * SCREEN_HEIGHT}),
			true,
			static_cast<unsigned char>(static_cast<unsigned char>(DEFAULT_CHARACTER_SIZE)),
			0,
			std::bind(&StartInterface::continueGame, this)
		};

		InterfaceElement new_game_button =
		{
			L"新游戏",
			sf::Vector2f({SCREEN_WIDTH / 2, 0.7 * SCREEN_HEIGHT}),
			true,
			static_cast<unsigned char>(static_cast<unsigned char>(DEFAULT_CHARACTER_SIZE)),
			0,
			std::bind(&StartInterface::confirmAndNewGame, this)
		};

		InterfaceElement quit_button =
		{
			L"退出游戏",
			sf::Vector2f({SCREEN_WIDTH / 2, 0.8 * SCREEN_HEIGHT}),
			true,
			static_cast<unsigned char>(DEFAULT_CHARACTER_SIZE),
			0,
			std::bind(&StartInterface::quit, this)
		};

		element_list.push_back(continue_game_button);
		element_list.push_back(new_game_button);
		element_list.push_back(quit_button);
	}
	init();
}

StartInterface::~StartInterface()
{
	
}

void StartInterface::continueGame()
{
	send(MessageQueue::game_code, "continue game");
}

void StartInterface::newGame()
{
	send(MessageQueue::game_code, "new game");
}

void StartInterface::confirmAndNewGame()
{
	std::string str;
	str = (std::string)u8"confirm:" + u8"选择新游戏将放弃当前进度" + u8"yes_message:" + u8"new game" + u8"no_message:" + u8"back to main page";
	send(MessageQueue::page_code, str);
}

void StartInterface::quit()
{
	send(MessageQueue::game_code, "quit");
}

ConfirmInterface::ConfirmInterface(const std::string confirm_message, const std::string i_yes_message, const std::string i_no_message)
{
	yes_message = i_yes_message;
	no_message = i_no_message;

	InterfaceElement confirm =
	{
		Script::stringToWstring(confirm_message),
		sf::Vector2f({SCREEN_WIDTH / 2, 0.6 * SCREEN_HEIGHT}),
		false,
		static_cast<unsigned char>(DEFAULT_CHARACTER_SIZE),
		0,
		nullptr
	};

	InterfaceElement yes_button =
	{
		L"确定",
		sf::Vector2f({SCREEN_WIDTH / 2, 0.7 * SCREEN_HEIGHT}),
		true,
		static_cast<unsigned char>(DEFAULT_CHARACTER_SIZE),
		0,
		std::bind(&ConfirmInterface::yes, this)
	};

	InterfaceElement no_button =
	{
		L"返回",
		sf::Vector2f({SCREEN_WIDTH / 2, 0.8 * SCREEN_HEIGHT}),
		true,
		static_cast<unsigned char>(DEFAULT_CHARACTER_SIZE),
		0,
		std::bind(&ConfirmInterface::no, this)
	};

	element_list.push_back(confirm);
	element_list.push_back(no_button);
	element_list.push_back(yes_button);

	init();
}

ConfirmInterface::~ConfirmInterface()
{

}

void ConfirmInterface::yes()
{
	send(MessageQueue::game_code, yes_message);
}

void ConfirmInterface::no()
{
	send(MessageQueue::page_code, no_message);
}

void DialogueInterface::initChoices()
{
	if (dialogue.choices.size() <= 0)
		return;

	int max_width = 0;

	for (int i = 1; i < element_list.size(); i++)
	{
		if (element_list.at(i).can_be_press)	//???????????
			button_num++;

		Text* text = new Text;
		text->setCharacterSize(element_list.at(i).character_size);
		text->setFont(element_list.at(i).font_id);
		text->setMessage(element_list.at(i).message);

		sf::Vector2f text_center =
		{
			position.x + element_list.at(i).center.x,
			position.y + element_list.at(i).center.y
		};
		text->setCenter(text_center);

		if (element_list.at(i).align)
		{
			if (text->getBackgroundWidth() > max_width)
			{
				max_width = text->getBackgroundWidth();
			}
		}

		text_list.push_back(text);
	}

	for (int i = 0; i < element_list.size(); i++)
	{
		if (element_list.at(i).align)
		{
			//??????
			text_list.at(i)->setBackgroundWidthForcely(max_width + CELL_SIZE * 3);
		}
	}

	//????????
	if (button_num > 0)
	{
		choose_box = new sf::RectangleShape();
		choose_box->setFillColor(sf::Color(0, 0, 0, 0));
		choose_box->setOutlineColor(sf::Color::White);
		choose_box->setOutlineThickness(1);
	}

	if (text_list.size() > 1)
	{
		auto choice_box_left_edge = text_list.at(1)->getGloabalBounds().left;
		float dialogue_box_left_edge = text_list.at(0)->getGloabalBounds().left;
		float delta = choice_box_left_edge - dialogue_box_left_edge;
		for (int i = 1; i < text_list.size(); i++)
		{
			element_list.at(i).center.x -= delta;
		}
	}
}


void DialogueInterface::react()
{
 	MessageQueue::Mail mail = check();
	if (mail.from == MessageQueue::blank_code)
		return;

	if (mail.message == "control is pressed")
	{
		skip_dialogue = true;
	}
	else if (mail.message == "control is released")
	{
		skip_dialogue = false;
	}

	if (dialogue_text_finish == false)
		return;

	if (button_num > 0)
	{
		if (mail.message == "up is pressed" || mail.message == "left is pressed")
		{
			if (delay == 0)
			{
				if (current_button > 0)
					current_button--;
				else
					current_button = button_num - 1;
				delay = INTERFACE_KEYBOARD_SELECT_DELAY;
			}
		}
		else if (mail.message == "down is pressed" || mail.message == "right is pressed")
		{
			if (delay == 0)
			{
				if (current_button < button_num - 1)
					current_button++;
				else
					current_button = 0;
				delay = INTERFACE_KEYBOARD_SELECT_DELAY;
			}
		}
		else if (mail.message == "enter is pressed" || mail.message == "space is pressed")
		{
			for (int i = 0; i < element_list.size(); i++)
			{
				if (element_list.at(i).can_be_press)
				{
					if (current_button == 0)
					{
						element_list.at(i).function();
						return;
					}
					else
					{
						current_button--;
					}
				}
			}
		}
	}
	else
	{
		if (mail.message == "enter is pressed" || mail.message == "space is pressed" || skip_dialogue)
		{
			std::stringstream message;
			message << "goto dialogue " << next_dialogue_id;
			send(MessageQueue::scene_code, message.str());
			if (dialogue.effect.size() > 0)
			{
				for (const auto& effect : dialogue.effect)
				{
					Script::parseEffect(effect);
				}
			}
		}
	}
}

DialogueInterface::DialogueInterface(Script::Dialogue i_dialogue, bool i_is_npc_dialogues)
{
	dialogue = i_dialogue;

	postal_code = MessageQueue::getPostalCode(MessageQueue::dialogue_interface_header);	//??д????

	next_dialogue_id = dialogue.next_dialogue;
	dialogue_delay = DIALOGUE_SHOW_CHARACTER_DELAY;
	dialogue_text_finish = false;
	is_npc_dialogues = i_is_npc_dialogues;

	skip_dialogue = false;

	std::wstring text;
	if (dialogue.speaker != L"")
		text = dialogue.speaker + L":\n";

	char_num = 0;
	InterfaceElement dialogue_text =
	{
		text,
		sf::Vector2f(SCREEN_WIDTH / 2, 6 * SCREEN_HEIGHT / 7),
		false,
		static_cast<unsigned char>(DEFAULT_DIALOGUE_CHARATER_SIZE),
		0,
		nullptr,
		false
	};

	element_list.push_back(dialogue_text);

	init();

	//强制对齐
	text_list.at(0)->setBackgroundWidthForcely(SCREEN_WIDTH - 50, 1);
}

void DialogueInterface::update(sf::RenderWindow* window)
{
	if (dialogue_text_finish == false)
	{
		//防连点
		if (dialogue_delay != 0)
		{
			dialogue_delay--;
			return;
		}

		if (char_num <= dialogue.text.size())
		{
			dialogue_delay = DIALOGUE_SHOW_CHARACTER_DELAY;
			short prev_char_num = char_num;

			if (!skip_dialogue)
				char_num += 1;
			else
			{
				char_num += 15;
			}
			
			//延迟出字
			std::wstring text = text_list.at(0)->getMessage();
			for (; prev_char_num < std::min(static_cast<size_t>(char_num), static_cast<size_t>(dialogue.text.size()));
				prev_char_num++)
			{
				if (text_list.at(0)->getTextBounds().width > 740)
					text.insert(std::max(text.size() - 2U, static_cast<size_t>(0)), L"\n");
				text += dialogue.text[prev_char_num];
				text_list.at(0)->setMessage(text);
			}
		}
		else
		{
			dialogue_text_finish = true;
			char_num--;
			if (dialogue.choices.size() > 0)
			{
				for (int i = 0; i < dialogue.choices.size(); i++)
				{
					auto& choice = dialogue.choices.at(i);
					std::stringstream message;
					message << "goto dialogue " << choice.next_dialogue;
					float height_factor = 0.5 + ((i + 0.5) / static_cast<float>(dialogue.choices.size() + 0.5)) / 3;	//????1/2??5/6???
					InterfaceElement choice_text =
					{
						dialogue.choices.at(i).text,
						sf::Vector2f(SCREEN_WIDTH / 5, SCREEN_HEIGHT * height_factor),
						true,
						static_cast<unsigned char>(DEFAULT_DIALOGUE_CHARATER_SIZE),
						0,
						std::bind([](auto ptr, std::string msg, const auto& choice) 
							{
							ptr->send(MessageQueue::scene_code, msg);
							if (choice.effect.size() > 0)
							{
								for (const auto& effect : choice.effect)
								{
									Script::parseEffect(effect);
								}
							}
							}, this, message.str(), choice)
					};
					element_list.push_back(choice_text);
				}

				initChoices();
			}
		}
	}
	else
	{
		if(button_num > 0)
		{
			//??????
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && window->hasFocus())
			{
				sf::Vector2i mousePosWindow = sf::Mouse::getPosition(*window);
				sf::Vector2f mousePosView = window->mapPixelToCoords(mousePosWindow);
				float scale_factor = view.getSize().x / SCREEN_WIDTH;
				sf::FloatRect choice_bounds;

				for (int i = 0; i < text_list.size(); i++)
				{
					if (element_list.at(i).can_be_press == false)
						continue;

					Text* text = text_list.at(i);
					choice_bounds = text->getGloabalBounds();
					choice_bounds.left *= scale_factor;
					choice_bounds.top *= scale_factor;
					choice_bounds.width *= scale_factor;
					choice_bounds.height *= scale_factor;
					if (choice_bounds.contains(mousePosView))
					{
						element_list.at(i).function();
					}
				}
			}

			if (delay > 0)
				delay--;

			setChooseBox();
		}
		
	}

	checkMailBoxAndReact();
	
}



const bool DialogueInterface::getIsNPCDialogues() const
{
	return is_npc_dialogues;
}