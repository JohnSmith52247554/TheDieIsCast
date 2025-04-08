#include "MovableWall.h"

MovableWall::MovableWall(int i_x, int i_y, ObjectManagement* i_object_management)
	: Object(i_x + CELL_SIZE / 2, i_y + CELL_SIZE / 2)
{
	i_object_management->pushBackObject(this);
	original_coord = coord;
	this->postal_code = MessageQueue::getPostalCode(MessageQueue::movable_wall_header);
}

MovableWall::MovableWall(int i_x, int i_y, float i_speed, short i_length, ObjectManagement* i_object_management)
	: MovableWall::MovableWall(i_x, i_y, i_object_management)
{
	speed = i_speed;
	length = i_length;
}

MovableWall::~MovableWall()
{
	short cell_x = bounds.left / CELL_SIZE;
	short cell_y = bounds.top / CELL_SIZE;

	MapCollision::deregisterCollisionObject(this);
}

void MovableWall::init(Direction i_structure_direction, Direction i_move_direction, short i_move_distance, bool set_can_be_affected_by_time_control)
{
	structure_direction = i_structure_direction;
	move_direction = i_move_direction;
	current_moving_direction = i_move_direction;
	move_distance = i_move_distance;

	this->can_be_affected_by_time_control = set_can_be_affected_by_time_control;

	//检查有效性
	const short cell_x = coord.x / CELL_SIZE;
	const short cell_y = coord.y / CELL_SIZE;
	if (structure_direction == left)
	{
		for (short begin_x = cell_x; begin_x > cell_x - length; begin_x--)
		{
			if (MapCollision::getCell(begin_x * CELL_SIZE, cell_y * CELL_SIZE) != Empty)
			{
				std::cout << "Spawn movable wall failed. Becouse " << begin_x << ' ' << cell_y << " isn't empty.";
				return;
			}
		}
	}
	else if (structure_direction == right)
	{
		for (short begin_x = cell_x; begin_x < cell_x + length; begin_x++)
		{
			if (MapCollision::getCell(begin_x * CELL_SIZE, cell_y * CELL_SIZE) != Empty)
			{
				std::cout << "Spawn movable wall failed. Becouse " << begin_x << ' ' << cell_y << " isn't empty.";
				return;
			}
		}
	}
	else if (structure_direction == up)
	{
		for (short begin_y = cell_y; begin_y > cell_y - length; begin_y--)
		{
			if (MapCollision::getCell(cell_x * CELL_SIZE, begin_y * CELL_SIZE) != Empty)
			{
				std::cout << "Spawn movable wall failed. Becouse " << cell_x << ' ' << begin_y << " isn't empty.";
				return;
			}
		}
	}
	else if (structure_direction == down)
	{
		for (short begin_y = cell_y; begin_y < cell_y + length; begin_y++)
		{
			if (MapCollision::getCell(cell_x * CELL_SIZE, begin_y * CELL_SIZE) != Empty)
			{
				std::cout << "Spawn movable wall failed. Becouse " << cell_x << ' ' << begin_y << " isn't empty.";
				return;
			}
		}
	}

	//根据长度和方向建立贴图
	//创建一个 RenderTexture，大小根据墙的总高度和宽度设置
	sf::RenderTexture render_texture;
	render_texture.create(length * CELL_SIZE, CELL_SIZE);

	//清空 RenderTexture
	render_texture.clear(sf::Color::Transparent);

	//先绘制向右延伸的墙，然后再进行旋转
	//绘制顶部部分
	sf::Sprite top_sprite(*ResourceManagement::getTexture(8));
	top_sprite.setTextureRect(sf::IntRect(0, 0, CELL_SIZE, CELL_SIZE));
	top_sprite.setPosition(0, 0);
	render_texture.draw(top_sprite);

	//绘制中部
	sf::Sprite middle_sprite(*ResourceManagement::getTexture(8));
	middle_sprite.setTextureRect(sf::IntRect(CELL_SIZE, 0, CELL_SIZE, CELL_SIZE));
	for (int i = 1; i < length - 1; i++)
	{
		middle_sprite.setPosition(CELL_SIZE * i, 0);
		render_texture.draw(middle_sprite);
	}

	//绘制尾部
	top_sprite.setRotation(180);
	top_sprite.setPosition(CELL_SIZE * length, CELL_SIZE);
	render_texture.draw(top_sprite);

	render_texture.display();

	this->texture = render_texture.getTexture();

	this->sprite.setTexture(texture);

	//统一旋转
	this->sprite.setOrigin(CELL_SIZE / 2, CELL_SIZE / 2);
	if (structure_direction == left)
	{
		this->sprite.setRotation(180);
	}
	else if (structure_direction == up)
	{
		this->sprite.setRotation(270);
	}
	else if (structure_direction == down)
	{
		this->sprite.setRotation(90);
	}

	//设置碰撞箱大小
	if (structure_direction == right || structure_direction == left)
	{ 
		hit_box.width = static_cast<float>(length) * static_cast<float>(CELL_SIZE) + 2;
		hit_box.height = static_cast<float>(CELL_SIZE) + 2;
	}
	else
	{
		hit_box.width = static_cast<float>(CELL_SIZE) + 2;
		hit_box.height = static_cast<float>(length) * static_cast<float>(CELL_SIZE) + 2;
	}
	bounds.width = hit_box.width - 2;
	bounds.height = hit_box.height - 2;
	getHitBox();
	getBounds();

	cell_width = bounds.width / CELL_SIZE;
	cell_heigth = bounds.height / CELL_SIZE;

	MapCollision::registerCollisionObject(this);
}

void MovableWall::update()
{
	//重置碰撞
	/*getHitBox();*/
	/*short cell_x = bounds.left / CELL_SIZE;
	short cell_y = bounds.top / CELL_SIZE;

	for (short i_x = cell_x; i_x < cell_x + cell_width; i_x++)
	{
		for (short i_y = cell_y; i_y < cell_y + cell_heigth; i_y++)
		{
			if (MapCollision::getCell(i_x, i_y) == Empty || MapCollision::getCell(i_x, i_y) == Cell::movableWall)
			{
				MapCollision::setCell(i_x, i_y, Empty);
			}
		}
	}*/

	//std::cout << MapCollision::getCell(0, 21) << "\n";
	sf::FloatRect collision_bounds;
	collision_bounds.width = bounds.width - 2;
	collision_bounds.height = bounds.height - 2;
	collision_bounds.left = bounds.left + 1;
	collision_bounds.top = bounds.top + 1;
	unsigned char collision = MapCollision::map_collision(&collision_bounds, true, this);

	//std::cout << (collision & (1 << 3)) << (collision & (1 << 2)) << (collision & (1 << 1)) << (collision & (1 << 0)) << std::endl;

	if (current_moving_direction == right)
	{
		vertical_speed = 0;
		horizontal_speed = speed;

		if ((collision & (1 << 1)))
		{
			if (move_direction != null)
				current_moving_direction = left;
			else
			{
				current_moving_direction = null;
				horizontal_speed = 0;
				vertical_speed = 0;
			}
		}
	}
	else if (current_moving_direction == left)
	{
		vertical_speed = 0;
		horizontal_speed = -speed;

		//std::cout << (collision & (1 << 0)) << '\n';
		if ((collision & (1 << 0)))
		{
			if (move_direction != null)
				current_moving_direction = right;
			else
			{
				current_moving_direction = null;
				horizontal_speed = 0;
				vertical_speed = 0;
			}
		}
	}
	else if (current_moving_direction == up)
	{
		vertical_speed = -speed;
		horizontal_speed = 0;
		if ((collision & (1 << 2)))
		{
			if (move_direction != null)
				current_moving_direction = down;
			else
			{
				current_moving_direction = null;
				horizontal_speed = 0;
				vertical_speed = 0;
			}
		}
	}
	else if (current_moving_direction == down)
	{
		vertical_speed = speed;
		horizontal_speed = 0;
		if ((collision & (1 << 3)))
		{
			if (move_direction != null)
				current_moving_direction = up;
			else
			{
				current_moving_direction = null;
				horizontal_speed = 0;
				vertical_speed = 0;
			}
		}
	}
	else if (current_moving_direction == null)
	{
		horizontal_speed = 0;
		vertical_speed = 0;
	}

	move();

	if (abs(coord.x - original_coord.x) + abs(coord.y - original_coord.y) >= move_distance * CELL_SIZE
		|| abs(coord.x - original_coord.x) + abs(coord.y - original_coord.y) <= 0)
	{
		if (move_direction != null)
		{
			if (current_moving_direction == right)
				current_moving_direction = left;
			else if (current_moving_direction == left)
				current_moving_direction = right;
			else if (current_moving_direction == up)
				current_moving_direction = down;
			else if (current_moving_direction == down)
				current_moving_direction = up;
		}
		else
		{
			current_moving_direction = null;
			horizontal_speed = 0;
			vertical_speed = 0;
		}
	}

	//设置碰撞
	getBounds();
	/*cell_x = bounds.left / CELL_SIZE;
	cell_y = bounds.top / CELL_SIZE;

	for (short i_x = cell_x; i_x < cell_x + cell_width; i_x++)
	{
		for (short i_y = cell_y; i_y < cell_y + cell_heigth; i_y++)
		{
			if (MapCollision::getCell(i_x, i_y) == Empty)
				MapCollision::setCell(i_x, i_y, Cell::movableWall);
		}
	}*/
	
	//只有设定为null的可以控制
	if (move_direction == null)
		checkMailBoxAndReact();
}

const sf::FloatRect* MovableWall::getHitBox()
{
	if (structure_direction == left)
	{
		hit_box.left = coord.x - CELL_SIZE / 2 - (length - 1) * CELL_SIZE - 1;
		hit_box.top = coord.y - CELL_SIZE / 2 - 1;
	}
	else if (structure_direction == up)
	{
		hit_box.left = coord.x - CELL_SIZE / 2 - 1;
		hit_box.top = coord.y - CELL_SIZE / 2 - (length - 1) * CELL_SIZE - 1;
	}
	else
	{
		hit_box.left = coord.x - CELL_SIZE / 2 - 1;
		hit_box.top = coord.y - CELL_SIZE / 2 - 1;
	}
	return &hit_box;
}

const sf::FloatRect* MovableWall::getBounds()
{
	if (structure_direction == left)
	{
		bounds.left = coord.x - CELL_SIZE / 2 - (length - 1) * CELL_SIZE;
		bounds.top = coord.y - CELL_SIZE / 2;
	}
	else if (structure_direction == up)
	{
		bounds.left = coord.x - CELL_SIZE / 2;
		bounds.top = coord.y - CELL_SIZE / 2 - (length - 1) * CELL_SIZE;
	}
	else
	{
		bounds.left = coord.x - CELL_SIZE / 2;
		bounds.top = coord.y - CELL_SIZE / 2;
	}
	return &bounds;
}

const unsigned short MovableWall::encode() const
{
	/*
	用三位来保存current_moving_direction
	四位来保存move_distance
	*/
	unsigned short output = 0U;

	output += current_moving_direction;

	output += (move_distance << 3);

	return output;
}

void MovableWall::decode(const unsigned short i_eigen_code)
{
	current_moving_direction = static_cast<Direction>(i_eigen_code & 7);
	move_distance = (i_eigen_code >> 3) & 15;
}

void MovableWall::setState(sf::Vector2f position, float i_horizontal_speed, float i_vertical_speed)
{
	short cell_x = bounds.left / CELL_SIZE;
	short cell_y = bounds.top / CELL_SIZE;

	/*for (short i_x = cell_x; i_x < cell_x + cell_width; i_x++)
	{
		for (short i_y = cell_y; i_y < cell_y + cell_heigth; i_y++)
		{
			if (MapCollision::getCell(i_x, i_y) == Empty)
			{
				MapCollision::setCell(i_x, i_y, Empty);
			}
		}
	}*/

	coord = position;
	horizontal_speed = i_horizontal_speed;
	vertical_speed = i_vertical_speed;

	getBounds();
	cell_x = bounds.left / CELL_SIZE;
	cell_y = bounds.top / CELL_SIZE;

	/*for (short i_x = cell_x; i_x < cell_x + cell_width; i_x++)
	{
		for (short i_y = cell_y; i_y < cell_y + cell_heigth; i_y++)
		{
			if (MapCollision::getCell(i_x, i_y) == Empty)
				MapCollision::setCell(i_x, i_y, Cell::movableWall);
		}
	}*/
}

void MovableWall::react()
{
	MessageQueue::Mail mail = check();

	if (mail.from == MessageQueue::blank_code)
		return;

	if (MessageQueue::computeInterclassicCode(mail.from) == MessageQueue::receptor_header)
	{
		std::stringstream ss(mail.message);
		std::string command, direcion;
		int distance;

		ss >> command >> direcion >> distance;

		if (command == "move")
		{
			current_moving_direction = static_cast<Direction>(MapNote::stringToEnum(direcion));
			move_distance = distance;
		}
		else if (command == "set")
		{
			original_coord = coord;
			current_moving_direction = static_cast<Direction>(MapNote::stringToEnum(direcion));
			move_distance = distance;
		}
	}
}