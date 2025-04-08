/*

玩家靠近时可以按F改变拉杆的状态
拉杆的状态不会自行改变
状态改变时发送激活/释放信号

*/

#pragma once

#include "Config.h"
#include "ObjectManagement.h"
#include "Receptor.h"
#include "MapNote.h"

class Drawbar : public Receptor
{
protected:
	bool hitted_by_player, f_is_pressed;

public:
	Drawbar(int i_x, int i_y, ObjectManagement* i_object_manager);
	Drawbar(const MapNote::ReceptorAndEffectors& receptor_info, ObjectManagement* i_object_manager);
	~Drawbar();

	virtual void update() override;

	virtual const sf::FloatRect* getHitBox() override;

	virtual void react() override;
};