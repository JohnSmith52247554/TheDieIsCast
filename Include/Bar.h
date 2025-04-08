#pragma once

#include "Config.h"
#include "ResourceManagement.h"
#include "Animation.h"

#include <sstream>

/*
屏幕上方的提示条
包含金币数等信息
TODO：生命值，关卡数等
*/

class Bar : public sf::Drawable, public sf::Transformable
{
	//变量
private:
	sf::Sprite spark_icon;
	sf::Text* spark_info;

	sf::Vector2f view_center;

	Animation* spark_fliping;

	mutable std::ostringstream six_digit_points;
	mutable std::ostringstream minutes_and_seconds;
	mutable std::stringstream info;

	mutable sf::Sprite bar_sprite;
	mutable sf::RenderTexture texture;

	//函数
public:
	Bar();
	~Bar();

	void updateViewCenter(sf::Vector2f center);

	void update();

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

