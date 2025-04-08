#include "Animation.h"

Animation::Animation(int ID, unsigned char total_frame1, sf::Sprite* i_sprite, unsigned char width, unsigned char height)
{
	texture_id = ID;
	current_frame = total_frame1 - 1;
	total_frame = total_frame1;
	timer = ANIMATION_SPEED;
	sprite = i_sprite;
	texture_width = width;
	texture_height = height;
	animation_speed = ANIMATION_SPEED;
}

void Animation::setAnimationSpeed(unsigned char speed)
{
	animation_speed = speed;
}

void Animation::reset()
{
	current_frame = 0;
	timer = 0;
	sf::IntRect textureRect(0, 0, texture_width, texture_height);
	sprite->setTextureRect(textureRect);
}

void Animation::setFrame()
{
	sprite->setTexture(*ResourceManagement::getTexture(texture_id));
	if (timer < animation_speed)
		timer++;
	else
	{
		timer = 0;
		if (current_frame < total_frame - 1)
			current_frame++;
		else
			current_frame = 0;

		sf::IntRect textureRect(current_frame * texture_width, 0, texture_width, texture_height);
		sprite->setTextureRect(textureRect);
	}
}

unsigned char Animation::getCurrentFrame() const
{
	return current_frame;
}

void Animation::setCurrentFrameForcely(const unsigned char frame)
{
	sprite->setTexture(*ResourceManagement::getTexture(texture_id));
	if (current_frame != frame && frame >= 0 && frame < total_frame - 1)
	{
		current_frame = frame;
		sf::IntRect textureRect(current_frame * texture_width, 0, texture_width, texture_height);
		sprite->setTextureRect(textureRect);
	}
}


void Animation::setFrameForcely(const unsigned char frame)
{
	sprite->setTexture(*ResourceManagement::getTexture(texture_id));
	if (frame >= 0 && frame < total_frame - 1)
	{
		current_frame = frame;
		sf::IntRect textureRect(current_frame * texture_width, 0, texture_width, texture_height);
		sprite->setTextureRect(textureRect);
	}
}