#pragma once

#include "Config.h"
#include "Object.h"

#include <array>

class Object;

class MapCollision
{
	//变量
private:
	static std::vector<std::vector<Cell>> cell_occupied;
	static sf::RectangleShape hit_box;
	static std::vector<Object*> collision_object;

	//函数
public:
	//构造析构
	MapCollision();
	~MapCollision();

	static void clear();

	//设置地图宽度
	static void setMapSize(unsigned short i_width, unsigned short i_height);

	//写入读取
	static void setCell(unsigned short i_x, unsigned short i_y, Cell i_set);
	static Cell getCell(float i_x, float i_y, bool care_collision_object = true, Object* collision_excuse = nullptr);

	//检测碰撞
	static unsigned char map_collision(float i_x, float i_y);
	static unsigned char map_collision(const sf::FloatRect* i_hit_box, bool care_movable_wall = true, Object* collision_excuse = nullptr);

	static bool object_collision(const sf::FloatRect* a_hit_box, const sf::FloatRect* b_hit_box);

	//显示碰撞箱
	static void draw_hit_box(sf::RenderTarget* i_window);

	//计算矩形碰撞箱与一个点的距离
	static float hitBoxDistance(const sf::FloatRect* hit_box, const sf::Vector2f point);
	//计算两个矩形碰撞箱之间的距离
	static float hitBoxDistance(const sf::FloatRect* a_hit_box, const sf::FloatRect* b_hit_box);

	static void registerCollisionObject(Object* ptr);
	static void deregisterCollisionObject(Object* ptr);
};

