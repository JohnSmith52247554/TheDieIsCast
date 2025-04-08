#include "Laser.h"

Laser::Laser(int i_x, int i_y, ObjectManagement* i_enemy_manager, Direction fly_direction)
	: Laser::Laser(i_x, i_y, i_enemy_manager, fly_direction, 0)
{
	death_counter = ENEMY_DEATH_REMAIN_TIME / 3;
	postal_code = MessageQueue::getPostalCode(MessageQueue::laser_header);
}

Laser::Laser(int i_x, int i_y, ObjectManagement* i_enemy_manager, Direction i_fly_direction, unsigned int i_postal_code)
	: Enemy(i_x, i_y, i_enemy_manager)
{
	fly_direction = i_fly_direction;

	postal_code = i_postal_code;

	initTexture(13);

	timer = 0;

	switch (fly_direction)
	{
	case left:
		horizontal_speed = -speed;
		sprite.setRotation(0);
		break;
	case right:
		horizontal_speed = speed;
		sprite.setRotation(180);
		break;
	case up:
		vertical_speed = -speed;
		sprite.setRotation(90);
		break;
	case down:
		vertical_speed = speed;
		sprite.setRotation(-90);
		break;
	default:
		break;
	}

	this->hit_box.width = sprite.getGlobalBounds().width;
	this->hit_box.height = sprite.getGlobalBounds().height;
	this->hit_box.left = i_x - sprite.getGlobalBounds().width / 2;
	this->hit_box.top = i_y - sprite.getGlobalBounds().height / 2;
}


Laser::~Laser()
{

}

void Laser::update()
{
	checkMailBoxAndReact();

	timer++;
	if (timer >= 1 * 60 * FPS)
	{
		should_update = true;
		return;
	}

	if (is_dead)
	{
		if (death_counter > 0)
			death_counter--;
		else
			should_update = false;
		return;
	}

	unsigned char collison = MapCollision::map_collision(getHitBox());

	switch (fly_direction)
	{
	case left:
		if (collison & (1 << 0))
			die();
		break;
	case right:
		if (collison & (1 << 1))
			die();
		break;
	case up:
		if (collison & (1 << 2))
			die();
		break;
	case down:
		if (collison & (1 << 3))
			die();
		break;
	default:
		break;
	}

	move();
}

const sf::FloatRect* Laser::getHitBox()
{
	this->hit_box.left = coord.x - sprite.getGlobalBounds().width / 2;
	this->hit_box.top = coord.y - sprite.getGlobalBounds().height / 2;

	return &hit_box;
}

const unsigned short Laser::encode() const
{
	unsigned short eigen_code = 0U;

	if (is_dead)
		eigen_code += 1;

	eigen_code += (fly_direction << 1);

	return eigen_code;
}

void Laser::decode(const unsigned short i_eigen_code)
{
	if ((i_eigen_code & (1 << 0)) == 0 && is_dead)
	{
		is_dead = false;
		switch (fly_direction)
		{
		case left:
			horizontal_speed = -speed;
			sprite.setRotation(0);
			break;
		case right:
			horizontal_speed = speed;
			sprite.setRotation(180);
			break;
		case up:
			vertical_speed = -speed;
			sprite.setRotation(90);
			break;
		case down:
			vertical_speed = speed;
			sprite.setRotation(-90);
			break;
		default:
			break;
		}
		death_counter = ENEMY_DEATH_REMAIN_TIME / 3;
	}
	else if ((i_eigen_code & (1 << 0)) > 0 && is_dead == false)
		die();

	if (fly_direction != (i_eigen_code >> 1))
	{
		fly_direction = static_cast<Direction>(i_eigen_code >> 1);
		switch (fly_direction)
		{
		case left:
			horizontal_speed = -speed;
			sprite.setRotation(0);
			break;
		case right:
			horizontal_speed = speed;
			sprite.setRotation(180);
			break;
		case up:
			vertical_speed = -speed;
			sprite.setRotation(90);
			break;
		case down:
			vertical_speed = speed;
			sprite.setRotation(-90);
			break;
		default:
			break;
		}
	}
	
}

void Laser::die()
{
	horizontal_speed = 0;
	vertical_speed = 0;
	is_dead = true;
}