#pragma once

#include "Object.h"
#include "ObjectManagement.h"

class Enemy : public Object
{
	//变量
protected:
	bool is_dead;

	unsigned short death_counter;	//用于在死亡一段时间后移除自身
	
	//函数
public:
	Enemy(int x, int y, ObjectManagement* i_enemy_manager);
	Enemy(int x, int y, ObjectManagement* i_enemy_manager, unsigned int i_postal_code);
	virtual ~Enemy() {};

protected:
	virtual void die() = 0;

public:
	const bool getIsDead() const;
	virtual void setIsDead(bool i_is_dead);

	virtual void update() = 0;

	//特征编码
	//编码
	virtual const unsigned short encode() const = 0;
	//解码
	virtual void decode(const unsigned short i_eigen_code) = 0;
};

