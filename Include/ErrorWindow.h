#pragma once

#include "Config.h"
#include "ResourceManagement.h"

class ErrorWindow
{
private:
	sf::RenderWindow window;
	sf::Text text;

public:
	ErrorWindow(bool error_or_warning, const char* error);

	void update();

	const bool isWindowOpen() const;
};