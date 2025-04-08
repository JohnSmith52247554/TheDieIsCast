#pragma once

#include "Config.h"
#include "Object.h"
#include "ObjectManagement.h"
#include "MapNote.h"

#include <algorithm>

class MovableWall : public Object
{
	//变量
private:
	float speed;
	short length;
	short move_distance;
	short cell_width;
	short cell_heigth;
	Direction structure_direction;
	Direction move_direction;

	sf::Texture texture;

	sf::Vector2f original_coord;	//起始的坐标
	Direction current_moving_direction;

	sf::FloatRect bounds;

	//函数
public:
	MovableWall(int i_x, int i_y, ObjectManagement* i_object_management);
	MovableWall(int i_x, int i_y, float i_speed, short i_length,ObjectManagement* i_object_management);
	~MovableWall();

	void init(Direction i_structure_direction, Direction i_move_direction, short i_move_distance, bool set_can_be_affected_by_time_control);

	virtual void update() override;

	virtual const sf::FloatRect* getHitBox() override;

	const sf::FloatRect* getBounds();

	//编码
	virtual const unsigned short encode() const override;
	//解码
	virtual void decode(const unsigned short i_eigen_code) override;

	virtual void setState(sf::Vector2f position, float i_horizontal_speed, float i_vertical_speed) override;

private:
	virtual void react() override;
};