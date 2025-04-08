#include "ErrorWindow.h"
#include <list>

ErrorWindow::ErrorWindow(bool error_or_warning, const char* error)
{
	ResourceManagement::cleanSounds();

	std::list<char> lst;
	int i = 0;
	while (error[std::max(i - 1, 0)] != '\0')
	{
		lst.push_back(error[i]);
		i++;
		if (i % 40 == 0)
			lst.push_back('\n');
	}
	std::string str(lst.begin(), lst.end());

	text.setString(str);
	text.setFont(*ResourceManagement::getFont(0));
	text.setFillColor(sf::Color::Black);
	text.setCharacterSize(32 * 4);
	text.setScale(0.25, 0.25);

	window.create(sf::VideoMode(std::max(text.getGlobalBounds().width + 50, 600.f),
		std::max(text.getGlobalBounds().height + 50, 100.f)), error_or_warning ? "ERROR" : "WARNING",
		sf::Style::Close | sf::Style::Titlebar);
	auto icon_path = EXE_DIR / "Resources" / "Icon" / "error.png";
	sf::Image icon;
	icon.loadFromFile(icon_path.string());
	window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

	text.setPosition(25, 20);

	window.clear(sf::Color::White);
	window.draw(text);
	window.display();

	ResourceManagement::playSound("Error");
}

void ErrorWindow::update()
{
	sf::Event ev;
	while (window.pollEvent(ev))
	{
		switch (ev.type)
		{
		case sf::Event::Closed:
			window.close();
			break;
		default:
			break;
		}
	}
}

const bool ErrorWindow::isWindowOpen() const
{
	return window.isOpen();
}