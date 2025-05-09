#include "Page.h"

Page::Page(sf::RenderWindow* window)
{
	i_interface = nullptr;
	background_image.setPosition(0, 0);
	this->postal_code = MessageQueue::page_code;

	view.setCenter(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	window->setView(view);

#if SHOW_CONSOLE
	cheating_mode = false;
#endif
}

Page::~Page()
{
	if (i_interface != nullptr)
		delete i_interface;
}

void Page::update(sf::RenderWindow* window)
{
	i_interface->update(window);
}

void Page::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(this->background_image, states);
	this->i_interface->draw(target, states);
}

const unsigned short Page::getInterfacePostalCode() const
{
	if (this->i_interface != nullptr)
		return i_interface->getPostalCode();
	else
		return 0U;
}

MainPage::MainPage(sf::RenderWindow* window)
	: Page::Page(window)
{
	window->setMouseCursorVisible(true);
	ResourceManagement::setBackgroundMusic("V_Everloop");
	i_interface = new StartInterface;

	auto save_dir = EXE_DIR / "History" / "save.bin";
	std::ifstream file(save_dir, std::ios::binary);
	if (file.is_open())
		file.read(reinterpret_cast<char*>(&ending_has_achieved), sizeof(ending_has_achieved));
	else
		ending_has_achieved = 0;
	file.close();

	std::filesystem::path file_dir;
	
	unsigned char flag = 0U;
	if (ending_has_achieved & (1 << 2) || ending_has_achieved & (1 << 3))
		flag += 1U;
	if (ending_has_achieved & (1 << 4) || ending_has_achieved & (1 << 5))
		flag += 2U;
	switch (flag)
	{
	case 0U:
		file_dir = EXE_DIR / "Resources" / "CG" / "MainPage.png";
		break;
	case 1U:
		file_dir = EXE_DIR / "Resources" / "CG" / "MainPage_NormalEnding.png";
		break;
	case 2U:
		file_dir = EXE_DIR / "Resources" / "CG" / "MainPage_TrueEnding.png";
		break;
	case 3U:
		file_dir = EXE_DIR / "Resources" / "CG" / "MainPage_Full.png";
		break;
	default:
		break;
	}
	texture.loadFromFile(file_dir.string());
	background_image.setTexture(texture);
	if (background_image.getGlobalBounds().width > SCREEN_WIDTH)
	{
		float factor = SCREEN_WIDTH / background_image.getGlobalBounds().width;
		background_image.setScale({ factor, factor });
	}
}

MainPage::~MainPage()
{
	ResourceManagement::stopBackgroundMusic();

	for (auto& item : items)
		if (item != nullptr)
			delete item;
}

void MainPage::update(sf::RenderWindow* window)
{
	i_interface->update(window);

	checkMailBoxAndReact();
}

void MainPage::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(this->background_image, states);

	for (auto& item : items)
		target.draw(*item, states);

	this->i_interface->draw(target, states);
}

void MainPage::react()
{
	MessageQueue::Mail mail = check();

	if (mail.from == MessageQueue::blank_code)
		return;

	if (MessageQueue::computeInterclassicCode(mail.from) == MessageQueue::interface_header)
	{
		if (mail.message == "back to main page")
		{
			if (i_interface != nullptr)
			{
				delete i_interface;
				i_interface = nullptr;
			}
			i_interface = new StartInterface;
			return;
		}

		std::string prefix = "confirm:";
		if (mail.message.substr(0, prefix.size()) == prefix)
		{
			// ??? "confirm:" ?? "yes_message:" ????????
			size_t confirm_start = mail.message.find("confirm:") + std::string("confirm:").length();
			size_t confirm_end = mail.message.find("yes_message:");
			std::string confirm_message = mail.message.substr(confirm_start, confirm_end - confirm_start);

			// ??? "yes_message:" ?? "no_message:" ????????
			size_t yes_start = confirm_end + std::string("yes_message:").length();
			size_t yes_end = mail.message.find("no_message:");
			std::string yes_message = mail.message.substr(yes_start, yes_end - yes_start);

			// ??? "no_message:" ?????????
			size_t no_start = yes_end + std::string("no_message:").length();
			std::string no_message = mail.message.substr(no_start);

			//??????????
			if (i_interface != nullptr)
			{
				delete i_interface;
				i_interface = nullptr;
			}

			i_interface = new ConfirmInterface(confirm_message, yes_message, no_message);
		}
	}
	else if (mail.from == MessageQueue::game_code)
	{
		if (i_interface != nullptr)
			send(i_interface->getPostalCode(), mail.message);
	}
}

EndingPage::EndingPage(sf::RenderWindow* window, const std::wstring& ending_name)
	: Page::Page(window)
{
	updateEndingHasAchieve(ending_name);

	speed = NORMAL_SPEED;

	ResourceManagement::setBackgroundMusic("TheCrabCanon");

	enum charSize
	{
		ending = 86,
		game_title = 50,
		team_name = 36,
		job = 28,
		person = 20
	};

	struct castInfo
	{
		const wchar_t* str;
		charSize size;
	};

	std::vector<castInfo> info =
	{
		{ ending_name.c_str(), ending },
		{ L"THE DIE IS CAST", game_title },
		{ L"Ave Mujica", team_name },
		{ L"程序", job },
		{ L"胡云淏", person },
		{ L"姜胜凯", person },
		{ L"文案", job },
		{ L"雷昱", person },
		{ L"美术", job },
		{ L"王岩", person },
		{ L"温旭", person },
		{ L"音乐", job },
		{ L"雷昱", person },
		{ L"特别鸣谢", job },
		{ L"拓竹科技", person },
		{ L"清华电子系学生科协软件部", person },
		{ L"deepseek", person },
		{ L"chatGPT", person },
		{ L"FittenCode", person },
		{ L"kimi", person }
	};

	cast.resize(info.size());

	unsigned int height = SCREEN_HEIGHT;

	for (unsigned short i = 0; i < cast.size(); i++)
	{
		auto& text = cast.at(i);
		text.setString(info.at(i).str);
		text.setCharacterSize(info.at(i).size * 4);
		text.setScale(0.25, 0.25);
		text.setFont(*ResourceManagement::getFont(0));
		text.setFillColor(sf::Color::White);
		
		auto bounds = text.getGlobalBounds();
		text.setPosition(
			SCREEN_WIDTH / 2 - bounds.width / 2,
			height
		);
		
		switch (info.at(i == info.size() - 1 ? i : i + 1).size)
		{
		case ending:
			height += bounds.height * 6;
			break;
		case game_title:
			height += bounds.height * 5;
			break;
		case team_name:
			height += bounds.height * 4;
			break;
		case job:
			height += bounds.height * 4;
			break;
		case person:
			height += bounds.height * 2;
			break;
		default:
			break;
		}
	}
}

EndingPage::~EndingPage()
{
	ResourceManagement::stopBackgroundMusic();
	update_task.get();
}

void EndingPage::update(sf::RenderWindow* window)
{
	checkMailBoxAndReact();

	/*if (cast.at(0).getPosition().y < SCREEN_HEIGHT)
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)
			|| sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
		{
			speed = -NORMAL_SPEED;
		}
		else
		{
			speed = NORMAL_SPEED;
		}
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::RControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
		speed = NORMAL_SPEED * 2;*/

	if (cast.at(cast.size() - 1).getPosition().y < -20)
		send(MessageQueue::game_code, "back to main page");

	moveCast();
}

void EndingPage::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	for (const auto text : cast)
		target.draw(text, states);
}

void EndingPage::react()
{
	MessageQueue::Mail mail = check();

	if (mail.from == MessageQueue::game_code)
	{
		if (mail.message == "shift is pressed")
		{
			if (cast.at(0).getPosition().y < SCREEN_HEIGHT)
				speed = -NORMAL_SPEED;
			else
				speed = 0;
		}
		else if (mail.message == "control is pressed")
		{
			speed = NORMAL_SPEED * 4;
		}
		else if (mail.message == "shift is released" || mail.message == "control is released")
		{
			speed = NORMAL_SPEED;
		}
	}
}

void EndingPage::moveCast()
{
	for (auto& text : cast)
		text.setPosition(text.getPosition().x, text.getPosition().y - speed);
}

void EndingPage::updateEndingHasAchieve(std::wstring ending_name)
{
	update_task = std::async(std::launch::async, [=](){
		//update
		unsigned short flag = 0U;
		if (ending_name == L"Bad Ending 1")
			flag = 0;
		else if (ending_name == L"Bad Ending 2")
			flag = 1;
		else if (ending_name == L"Normal Ending 1")
			flag = 2;
		else if (ending_name == L"Normal Ending 2")
			flag = 3;
		else if (ending_name == L"True Ending 1")
			flag = 4;
		else if (ending_name == L"True Ending 2")
			flag = 5;
		if (!(ending_has_achieved & (1 << flag)))
			ending_has_achieved += (1 << flag);

		auto save_file_dir = EXE_DIR / "History" / "save.bin";
		std::fstream file(save_file_dir, std::ios::binary | std::ios::in | std::ios::out);
		if (!file.is_open())
			throw std::logic_error("open save file failed");

		file.seekp(0, std::ios::beg);

		file.write(reinterpret_cast<char*>(&ending_has_achieved), sizeof(ending_has_achieved));

		file.close();
		});
}