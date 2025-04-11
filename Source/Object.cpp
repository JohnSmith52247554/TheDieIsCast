#include "Object.h"

Object::Object()
{
	this->coord.x = 0;
	this->coord.y = 0;

	horizontal_speed = 0.f;
	vertical_speed = 0.f;

	can_be_affected_by_time_control = true;

	should_update = true;

	reference_system_horizontal_speed = 0.f;
	reference_system_vertical_speed = 0.f;

	interactivable = false;

	stand_on_movable_wall = false;
	movable_wall_up_edge = 0.f;
}

Object::Object(int x, int y)
	: Object::Object()
{
	this->coord.x = x;
	this->coord.y = y;
}

Object::Object(int x, int y, unsigned int i_postal_code)
	: Object::Object(x, y)
{
	this->postal_code = i_postal_code;
}

void Object::initTexture(int ID)
{
	std::shared_ptr<sf::Texture> t = ResourceManagement::getTexture(ID);
	if (t != nullptr && t->getSize().x > 0 && t->getSize().y > 0)
	{
		this->sprite.setTexture(*t);
		sf::Vector2f center(
			t->getSize().x / 2.0f,
			t->getSize().y / 2.0f
		);
		sprite.setOrigin(center);
	}
	else
	{
		std::cout << "init texture failed, ID: " << ID << std::endl;
	}
}

const sf::Vector2f Object::getCoord() const
{
	return coord;
}

float Object::getVerticalSpeed() const
{
	return vertical_speed;
}

float Object::getHorizontalSpeed() const
{
	return horizontal_speed;
}

const bool Object::canBeAffectedByTimeControl() const
{
	return can_be_affected_by_time_control;
}

void Object::setPosition(int x, int y)
{
	this->coord.x = x;
	this->coord.y = y;
}

void Object::setCanBeAffectedByTimeControl(bool cbabtc)
{
	can_be_affected_by_time_control = cbabtc;
}

void Object::setState(sf::Vector2f position, float i_horizontal_speed, float i_vertical_speed)
{
	coord = position;
	horizontal_speed = i_horizontal_speed;
	vertical_speed = i_vertical_speed;

	sprite.setPosition(coord);
}

void Object::setReferenceSystem(float i_horizontal_speed, float i_vertical_speed)
{
	reference_system_horizontal_speed = i_horizontal_speed;
	reference_system_vertical_speed = i_vertical_speed;
}

void Object::setReferenceSystem(float i_horizontal_speed, float i_vertical_speed, bool on_the_wall, float wall_up_edge)
{
	reference_system_horizontal_speed = i_horizontal_speed;
	reference_system_vertical_speed = i_vertical_speed;
	stand_on_movable_wall = on_the_wall;
	movable_wall_up_edge = wall_up_edge;
}

sf::Sprite* Object::getSprite()
{
	this->sprite.setPosition(this->coord);
	return &sprite;
}

void Object::move()
{
	coord.x += horizontal_speed;
	coord.y += vertical_speed;
}

void Object::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	this->sprite.setPosition(coord);
	target.draw(this->sprite, states);

	/*sf::RectangleShape rect({ 2, 2 });
	rect.setPosition(coord.x - 1, coord.y - 1);
	rect.setFillColor(sf::Color::Red);
	target.draw(rect);*/
}

const bool Object::getShouldUpdate() const
{
	return should_update;
}

const bool Object::getInteractivable() const
{
	return interactivable;
}

const unsigned short Object::saveCode()
{
	return encode();
}

void Object::praseSaveCode(const unsigned short save_code)
{
	decode(save_code);
}