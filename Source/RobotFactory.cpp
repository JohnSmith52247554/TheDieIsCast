#include "RobotFactory.h"

RobotFactory::RobotFactory(int x, int y, ObjectManagement* object_manager, unsigned char flip_code)
	: Object(x, y)
{
	object_manager->pushBackObject(this);

	postal_code = MessageQueue::getPostalCode(MessageQueue::robot_factory_header);

	initTexture(15);
	this->sprite.setTextureRect({ 0, 0, 64, 96 });
	this->sprite.setOrigin(32, 48);

	generate_counter = GENERATE_ROBOT_DELAY;

	save_manager = object_manager;

	hit_box.width = 64;
	hit_box.height = 96;
	hit_box.left = coord.x - 32;
	hit_box.top = coord.y - 48;

	flip = flip_code;
	flipSprite(flip_code);
}

RobotFactory::~RobotFactory()
{

}

void RobotFactory::update()
{
	checkMailBoxAndReact();

	generate_counter--;

	if (generate_counter == 3 * GENERATE_ROBOT_DELAY / 4)
	{
		sprite.setTextureRect({ 0, 0, 64, 96 });
	}
	else if (generate_counter == 0)
	{
		auto r = new Robot(coord.x - 32, coord.y - 16, save_manager);
		r->setCanBeAffectedByTimeControl(can_be_affected_by_time_control);
		if (flip == static_cast<unsigned char>(0))
			r->setWalkDirection(right);
		else
			r->setWalkDirection(left);
		r->setEdgeCheck(false);
		generate_counter = GENERATE_ROBOT_DELAY;
		sprite.setTextureRect({ 64, 0, 64, 96 });

		sf::FloatRect view_port = { view.getCenter().x - view.getSize().x / 2,
				view.getCenter().y - view.getSize().y / 2, view.getSize().x, view.getSize().y };
		if (MapCollision::object_collision(&hit_box, &view_port))
			ResourceManagement::playSound("GenRobot");
	}
}

const sf::FloatRect* RobotFactory::getHitBox()
{
	return &hit_box;
}

const unsigned short RobotFactory::encode() const
{
	/*
	0-7: generate_counter	
	8-10: filp
	11: can_be_affected_by_time_control
	*/
	return generate_counter + ((flip & 7) << 8) + (can_be_affected_by_time_control << 11);
}

void RobotFactory::decode(const unsigned short i_eigen_code)
{
	generate_counter = i_eigen_code & 255;
	//std::cout << static_cast<short>(generate_counter) << std::endl;
	if (generate_counter >= 3 * GENERATE_ROBOT_DELAY / 4 && generate_counter >= 3 * GENERATE_ROBOT_DELAY / 4 + 1.5 * RECORD_DENSITY)
	{
		sprite.setTextureRect({ 64, 0, 64, 96 });
	}
	else if (generate_counter >= 0 && generate_counter <= 1.5 * RECORD_DENSITY)
	{
		sprite.setTextureRect({ 0, 0, 64, 96 });
	}

	if (flip != ((i_eigen_code >> 8) & 7))
	{
		flip = ((i_eigen_code >> 8) & 7);
		flipSprite(flip);
	}

	can_be_affected_by_time_control = (i_eigen_code >> 11) & 1;
}

void RobotFactory::flipSprite(const unsigned char code)
{
	if (code == 4)
		sprite.setScale(-1, 1);
}
