/*

玩家或敌人靠近时会转为激活状态
远离时自动转为释放状态
状态变换时发送信号

*/

#pragma once

#include "Config.h"
#include "ObjectManagement.h"
#include "Receptor.h"
#include "MapNote.h"

class PressurePlate : public Receptor
{
private:
	bool locked;

public:
	PressurePlate(const MapNote::ReceptorAndEffectors& receptor_info, ObjectManagement* i_object_manager);
	~PressurePlate();

	virtual void update() override;

	virtual const sf::FloatRect* getHitBox() override;

	virtual void react() override;
};