#pragma once

#include "Config.h"
#include "Laser.h"
#include "Object.h"
#include "ObjectManagement.h"
#include "Animation.h"

class LaserGun : public Object
{
private:
	Direction shoot_direction;

	Animation* shoot_animation;

	ObjectManagement* save_object_manager;

	unsigned char flip_code;

	bool can_shoot;

	unsigned char shoot_delay;
	const unsigned char SHOOT_DELAY = 40;

public:
	LaserGun(int i_x, int i_y, ObjectManagement* i_object_manager, unsigned char direcion_code);
	LaserGun(int i_x, int i_y, ObjectManagement* i_object_manager, unsigned char direcion_code, unsigned int i_postal_code);
	~LaserGun();

	virtual void update() override;

	virtual const sf::FloatRect* getHitBox() override;

	virtual const unsigned short encode() const override;
	virtual void decode(const unsigned short i_eigen_code) override;

private:
	void setFlip(unsigned short code);

};