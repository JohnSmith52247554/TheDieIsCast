#pragma once

#include "Enemy.h"
#include "MapCollision.h"
#include "TimeManagement.h"

class Robot : public Enemy
{
	//变量
private:
	Animation* robot_walking;
	Direction walk_direcion;

	bool edge_check;

	//函数
public:
	Robot(int x, int y, ObjectManagement* i_enemy_manager);
	Robot(int x, int y, ObjectManagement* i_enemy_manager, unsigned int i_postal_code);
	virtual ~Robot() override;

	virtual void update() override;

	virtual const sf::FloatRect* getHitBox() override;

	virtual void setIsDead(bool i_is_dead) override;

	//特征编码
	//编码
	virtual const unsigned short encode() const override;
	//解码
	virtual void decode(const unsigned short i_eigen_code) override;

	void setWalkDirection(Direction dir);
	void setEdgeCheck(const bool check);

private:
	//处理邮件
	virtual void react() override;

	virtual void die() override;
};

