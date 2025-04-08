#pragma once

#include "Config.h"
#include "Object.h"
#include "Robot.h"
#include "ObjectManagement.h"

class RobotFactory : public Object
{
private:
	const unsigned char GENERATE_ROBOT_DELAY = 180;
	unsigned char generate_counter;

	ObjectManagement* save_manager;
	unsigned char flip;


public:
	RobotFactory(int x, int y, ObjectManagement* object_manager, unsigned char flip_code);
	~RobotFactory();

	virtual void update() override;

	virtual const sf::FloatRect* getHitBox() override;

	virtual const unsigned short encode() const override;
	virtual void decode(const unsigned short i_eigen_code) override;

private:
	void flipSprite(const unsigned char code);
};