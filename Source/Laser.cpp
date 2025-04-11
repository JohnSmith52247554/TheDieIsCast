#include "Laser.h"

Laser::Laser(int i_x, int i_y, ObjectManagement* i_enemy_manager, Direction fly_direction)
	: Laser::Laser(i_x, i_y, i_enemy_manager, fly_direction, 0)
{
	death_counter = ENEMY_DEATH_REMAIN_TIME / 5;
	postal_code = MessageQueue::getPostalCode(MessageQueue::laser_header);
}

Laser::Laser(int i_x, int i_y, ObjectManagement* i_enemy_manager, Direction i_fly_direction, unsigned int i_postal_code)
	: Enemy(i_x, i_y, i_enemy_manager)
{
	fly_direction = i_fly_direction;

	postal_code = i_postal_code;

	laser_fly = new Animation(13, 8, &this->sprite, 8, 32);
	laser_fly->setAnimationSpeed(5);

	timer = 0;

	sprite.setOrigin({ 4, 16 });

	flip();
	this->hit_box.left = i_x + x_offset;
	this->hit_box.top = i_y + y_offset;
}


Laser::~Laser()
{
	delete laser_fly;
}

void Laser::update()
{
	checkMailBoxAndReact();

	laser_fly->setFrame();

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
	this->hit_box.left = coord.x + x_offset;
	this->hit_box.top = coord.y + y_offset;

	return &hit_box;
}

const unsigned short Laser::encode() const
{
	unsigned short eigen_code = 0U;

	if (is_dead)
		eigen_code += 1;

	eigen_code += (fly_direction << 1);
	eigen_code += (laser_fly->getCurrentFrame() << 3);

	return eigen_code;
}

void Laser::decode(const unsigned short i_eigen_code)
{
	if ((i_eigen_code & (1 << 0)) == 0 && is_dead)
	{
		is_dead = false;
		flip();
		death_counter = ENEMY_DEATH_REMAIN_TIME / 5;
	}
	else if ((i_eigen_code & (1 << 0)) > 0 && is_dead == false)
		die();

	if (fly_direction != ((i_eigen_code >> 1) & 3))
	{
		fly_direction = static_cast<Direction>((i_eigen_code >> 1) & 3);
		flip();
	}
	
	laser_fly->setCurrentFrameForcely((i_eigen_code >> 3) & 255);
}

void Laser::die()
{
	horizontal_speed = 0;
	vertical_speed = 0;
	is_dead = true;
}

void Laser::flip()
{
	sf::FloatRect bounds({ 0, 0, 8, 32 });
	switch (fly_direction)
	{
	case left:
		horizontal_speed = -speed;
		sprite.setRotation(90);
		x_offset = -bounds.height / 2;
		y_offset = -bounds.width / 2;
		break;
	case right:
		horizontal_speed = speed;
		sprite.setRotation(270);
		x_offset = -bounds.height / 2;
		y_offset = -bounds.width / 2;
		break;
	case up:
		vertical_speed = -speed;
		sprite.setRotation(0);
		x_offset = -bounds.width / 2;
		y_offset = -bounds.height / 2;
		break;
	case down:
		vertical_speed = speed;
		sprite.setRotation(180);
		x_offset = -bounds.width / 2;
		y_offset = -bounds.height / 2;
		break;
	default:
		break;
	}
	this->hit_box.width = sprite.getGlobalBounds().width;
	this->hit_box.height = sprite.getGlobalBounds().height;
}