#pragma once

#include "Config.h"
#include "ResourceManagement.h"

class Animation
{
	//变量
private:
	sf::Sprite* sprite;
	int texture_id;
	unsigned char texture_width;
	unsigned char texture_height;
	unsigned char current_frame;
	unsigned char total_frame;
	unsigned char timer;
	unsigned char animation_speed;

	//函数
public:
	Animation(int id, unsigned char total_frame1, sf::Sprite* i_sprite, unsigned char width, unsigned char height);
	
	void setAnimationSpeed(unsigned char speed);
	void reset();
	void setFrame();

	unsigned char getCurrentFrame() const;
	void setCurrentFrameForcely(const unsigned char frame);

	void setFrameForcely(const unsigned char frame);
};

