#include "MapCollision.h"

std::vector<std::vector<Cell>> MapCollision::cell_occupied;

sf::RectangleShape MapCollision::hit_box;

std::vector<Object*> MapCollision::collision_object;

MapCollision::MapCollision()
{
	for (auto& arr : this->cell_occupied)
	{
		arr.resize(1);
		for (Cell& c : arr)
		{
			c = Empty;
		}
	}
}

MapCollision::~MapCollision()
{

}

void MapCollision::clear()
{
	cell_occupied.clear();
}

void MapCollision::setMapSize(unsigned short i_width, unsigned short i_height)
{
	cell_occupied.resize(i_width);
	
	for (auto& arr : cell_occupied)
	{
		arr.resize(i_height);
		for (Cell& c : arr)
		{
			c = Empty;
		}
	}
}

void MapCollision::setCell(unsigned short i_x, unsigned short i_y, Cell i_set)
{
	if (i_x < 0 || i_x >= cell_occupied.size() || i_y < 0 || i_y >= cell_occupied.at(0).size())
		return;
	cell_occupied[i_x][i_y] = i_set;
}

Cell MapCollision::getCell(float i_x, float i_y, bool care_collision_object, Object* collision_excuse)
{
	if (i_x <= 0 || i_x >= cell_occupied.size() * CELL_SIZE || i_y <= 0 || i_y >= cell_occupied[0].size() * CELL_SIZE)
		return Wall;
	short cell_x = i_x / CELL_SIZE;
	short cell_y = i_y / CELL_SIZE;
	auto result = cell_occupied.at(cell_x).at(cell_y);
	if (result == Empty)
	{
		for (auto ptr : collision_object)
		{
			if (ptr == collision_excuse)
				continue;
			if (ptr->getHitBox()->contains(sf::Vector2f{ i_x, i_y }))
			{
				result = Wall;
				break;
			}
		}
	}
	return result;
}

//使用一个八位二进制数作为输出，每一位的情况代表一个方向的情况
unsigned char MapCollision::map_collision(float i_x, float i_y)
{
	//将准确坐标转化为对应的格子坐标
	float cell_x = i_x / CELL_SIZE;
	float cell_y = i_y / CELL_SIZE;

	hit_box.setPosition({ floor(cell_x) * CELL_SIZE, floor(cell_y) * CELL_SIZE });

	//初始化结果
	unsigned char output = 0;

	//第五位检测是否碰到地刺
	if (getCell(i_x, i_y, false) == Thorn)
		output += (1 << 4);

	for (unsigned char a = 0; a < 4; a++)
	{
		short x;
		short y;

		//遍历目标周围的四个格子
		switch (a)
		{
		case 0: //Top left cell
		{
			x = floor(cell_x);
			y = floor(cell_y);

			break;
		}
		case 1: //Top right cell
		{
			x = ceil(cell_x);
			y = floor(cell_y);

			break;
		}
		case 2: //Bottom left cell
		{
			x = floor(cell_x);
			y = ceil(cell_y);

			break;
		}
		case 3: //Bottom right cell
		{
			x = ceil(cell_x);
			y = ceil(cell_y);
		}
		}

		if (0 <= x && x <= cell_occupied.size())
		{
			if (0 <= y && y <= cell_occupied[0].size())
			{
				if (getCell(x * CELL_SIZE, y * CELL_SIZE) == Wall)
				{
					output += pow(2, a);
				}
			}
		}
		//将地图边缘视为墙壁（不可通行）
		else
		{
			output += pow(2, a);
		}
	}

	return output;
}

unsigned char MapCollision::map_collision(const sf::FloatRect* i_hit_box, bool care_movable_wall, Object* collision_excuse)
{
	//float up_left_x = i_hit_box->getPosition().x;
	//float up_left_y = i_hit_box->getPosition().y;
	//short hit_box_width = i_hit_box->getSize().x;
	//short hit_box_height = i_hit_box->getSize().y;

	//unsigned char output = 0;

	//short cell_x;
	//short cell_y;

	//for (int i = 0; i < 4; i++)
	//{
	//	
	//	if (i == 0 || i == 1)
	//	{
	//		for (int j = 0; j < 2; j++)
	//		{
	//			cell_y = static_cast<short>(floor((up_left_y + (static_cast<float>(hit_box_height) / 2) * j) / CELL_SIZE));
	//			//left
	//			if (i == 0)
	//			{
	//				cell_x = static_cast<short>(floor((up_left_x) / CELL_SIZE));
	//			}
	//			//right
	//			else
	//			{
	//				cell_x = static_cast<short>(floor((up_left_x + static_cast<float>(hit_box_width)) / CELL_SIZE));
	//			}
	//			if (0 <= cell_y && cell_y <= cell_occupied[0].size())
	//			{
	//				if (getCell(cell_x, cell_y) == Wall)
	//				{
	//					output += pow(2, i);
	//					break;
	//				}
	//			}
	//		}
	//	}

	//	else
	//	{
	//		for (int j = 0; j < 2; j++)
	//		{
	//			cell_x = static_cast<short>(floor((up_left_x + j * hit_box_width) / CELL_SIZE));
	//			//up
	//			if (i == 2)
	//			{
	//				cell_y = static_cast<short>(floor(up_left_y / CELL_SIZE));
	//			}
	//			//down
	//			else
	//			{
	//				cell_y = static_cast<short>(floor((up_left_y + hit_box_height) / CELL_SIZE));
	//			}
	//			if (0 <= cell_y && cell_y <= cell_occupied[0].size())
	//			{                      
	//				if (getCell(cell_x, cell_y) == Wall)
	//				{
	//					output += pow(2, i);
	//					break;
	//				}
	//			}
	//		}
	//	}
	//}

	//return output;

	unsigned char output = 0U;
	float up_left_x = i_hit_box->left;
	float up_left_y = i_hit_box->top;
	float hit_box_width = i_hit_box->width;
	float hit_box_height = i_hit_box->height;
	
	float test_x, test_y;
	//left
	test_x = up_left_x;
	for (short i = 0; i < std::max(hit_box_height, 1.f); i += 16)
	{
		//cell_y = floor((up_left_y) / static_cast<float>(CELL_SIZE)) + i;
		test_y = up_left_y + i;
		if (getCell(test_x, test_y, care_movable_wall, collision_excuse) == Wall)
		{
			output += 1;
			break;
		}
	}

	//right
	test_x = up_left_x + hit_box_width;
	for (short i = 0; i < std::max(hit_box_height, 1.f); i += 16)
	{
		//cell_y = floor((up_left_y) / static_cast<float>(CELL_SIZE)) + i;
		test_y = up_left_y + i;
		if (getCell(test_x, test_y, care_movable_wall, collision_excuse) == Wall)
		{
			output += 2;
			break;
		}
	}

	//up
	//cell_y = floor(up_left_y / static_cast<float>(CELL_SIZE));
	test_y = up_left_y;
	for (short i = 0; i <= hit_box_width; i += 16)
	{
		//cell_x = floor((up_left_x) / static_cast<float>(CELL_SIZE)) + i;
		test_x = up_left_x + i;
		if (getCell(test_x, test_y, care_movable_wall, collision_excuse) == Wall)
		{
			output += 4;
			break;
		}
	}

	//down
	//cell_y = floor((up_left_y + hit_box_height) / static_cast<float>(CELL_SIZE));
	test_y = up_left_y + hit_box_height;
	for (short i = 0; i <= hit_box_width; i += 16)
	{
		//cell_x = floor((up_left_x) / static_cast<float>(CELL_SIZE)) + i;
		test_x = up_left_x + i;
		if (getCell(test_x, test_y, care_movable_wall, collision_excuse) == Wall)
		{
			output += 8;
			break;
		}
	}

	/*cell_x = floor((up_left_x) / static_cast<float>(CELL_SIZE));
	cell_y = floor((up_left_y) / static_cast<float>(CELL_SIZE));*/
	test_x = up_left_x;
	test_y = up_left_y;
	float final_x = up_left_x + hit_box_width;
	float final_y = up_left_y + hit_box_height;
	for (test_x; test_x < final_x; test_x += 16)
	{
		for (; test_y < final_y; test_y += 16)
		{
			if (getCell(test_x, test_y, care_movable_wall, collision_excuse) == Thorn)
			{
				output += 16;
				return output;
			}
		}
	}

	return output;

}

bool MapCollision::object_collision(const sf::FloatRect* a_hit_box, const sf::FloatRect* b_hit_box)
{
	sf::FloatRect intersection;
	return a_hit_box->intersects(*b_hit_box, intersection);
}

void MapCollision::draw_hit_box(sf::RenderTarget* i_window)
{
	MapCollision::hit_box.setSize({ 2 * CELL_SIZE, 2 * CELL_SIZE });
	MapCollision::hit_box.setFillColor(sf::Color(0, 0, 0, 0));
	MapCollision::hit_box.setOutlineColor(sf::Color::Cyan);
	MapCollision::hit_box.setOutlineThickness(1);
	i_window->draw(hit_box);
}

float MapCollision::hitBoxDistance(const sf::FloatRect* hit_box, const sf::Vector2f point)
{
	if (hit_box->contains(point))
		return 0.f;

	float horizontal_distance = std::min(abs(hit_box->left - point.x), abs(hit_box->left + hit_box->width - point.x));
	float vertical_distance = std::min(abs(hit_box->top - point.y), abs(hit_box->top + hit_box->height - point.y));

	return sqrt(horizontal_distance * horizontal_distance + vertical_distance * vertical_distance);
}

float MapCollision::hitBoxDistance(const sf::FloatRect* a_hit_box, const sf::FloatRect* b_hit_box)
{
	if (object_collision(a_hit_box, b_hit_box))
		return 0.f;

	float horizontal_distance = std::min(abs(a_hit_box->left - (b_hit_box->left + b_hit_box->width)), abs(b_hit_box->left - (a_hit_box->left + a_hit_box->width)));
	float vertical_distance = std::min(abs(a_hit_box->top - (b_hit_box->top+ b_hit_box->height)), abs(b_hit_box->top - (a_hit_box->top + a_hit_box->height)));

	return sqrt(horizontal_distance * horizontal_distance + vertical_distance * vertical_distance);
}

void MapCollision::registerCollisionObject(Object* ptr)
{
	collision_object.push_back(ptr);
}

void MapCollision::deregisterCollisionObject(Object* ptr)
{
	collision_object.erase(std::remove_if(collision_object.begin(), collision_object.end(), [=](Object* check) {return check == ptr; }),
		collision_object.end());
}