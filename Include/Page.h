/*

带有背景图片的interface

*/

#pragma once

#include "Config.h"
#include "Interface.h"
#include "MessageQueueSubscriber.h"
#include "File.h"

class Page : public sf::Drawable, public sf::Transformable, public MessageQueueSubscriber
{
protected:
	Interface* i_interface;
	sf::Sprite background_image;
	sf::Texture texture;

public:
	Page(sf::RenderWindow* window);
	~Page();

	virtual void update(sf::RenderWindow* window);

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	const unsigned short getInterfacePostalCode() const;
};

class MainPage : public Page
{
public:
	MainPage(sf::RenderWindow* window);
	~MainPage();

	virtual void update(sf::RenderWindow* window) override;

private:
	virtual void react() override;
};

class EndingPage : public Page
{
private:
	std::vector<sf::Text> cast;
	float speed;
	const float NORMAL_SPEED = 0.3f;

public:
	EndingPage(sf::RenderWindow* window, const std::wstring& ending_name);
	~EndingPage();

	virtual void update(sf::RenderWindow* window) override;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	virtual void react() override;
	void moveCast();
};