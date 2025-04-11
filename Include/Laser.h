#pragma once

#include "Config.h"
#include "Enemy.h"
#include "ObjectManagement.h"
#include "Animation.h"

class Laser : public Enemy
{
private:
	const float speed = 1.9f;
	Direction fly_direction;

	unsigned int timer;	//������һ��ʱ����Ƴ���������ֹ�ѻ�

	Animation* laser_fly;

	int x_offset, y_offset;

public:
	Laser(int i_x, int i_y, ObjectManagement* i_enemy_manager, Direction fly_direction);
	Laser(int i_x, int i_y, ObjectManagement* i_enemy_manager, Direction fly_direction, unsigned int i_postal_code);
	~Laser();

	virtual void update() override;

	virtual const sf::FloatRect* getHitBox() override;

	virtual const unsigned short encode() const override;
	virtual void decode(const unsigned short i_eigen_code) override;

private:
	virtual void die() override;

	void flip();

};
