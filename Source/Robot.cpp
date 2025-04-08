#include "Robot.h"

Robot::Robot(int x, int y, ObjectManagement* i_enemy_manager)
	: Robot::Robot(x + 32, y + 32, i_enemy_manager, 0)
{
	this->postal_code = MessageQueue::getPostalCode(MessageQueue::robot_header);
}

Robot::Robot(int x, int y, ObjectManagement* i_enemy_manager, unsigned int i_postal_code)
	: Enemy(x, y, i_enemy_manager)
{
	robot_walking = new Animation(4, 13, this->getSprite(), 64, 64);
	robot_walking->setFrame();
	robot_walking->setAnimationSpeed(4);

	walk_direcion = left;

	this->hit_box.width = 48;
	this->hit_box.height = 25;

	this->postal_code = i_postal_code;

	sprite.setOrigin(32, 32);

	this->edge_check = true;
}

Robot::~Robot()
{
	delete robot_walking;

	if (!can_be_affected_by_time_control)
		TimeManagement::revive_black_list.insert(postal_code);
}

void Robot::die()
{
	death_counter = ENEMY_DEATH_REMAIN_TIME;

	is_dead = true;
	initTexture(5);
	
	//恢复正常的范围
	//避免由于动画中调整范围导致越界
	sprite.setTextureRect(sf::IntRect(0, 0, 64, 64));
	
	sf::Vector2f origin(32.0f, 32.0f);
	sprite.setOrigin(origin);

	if (walk_direcion == right)
		sprite.setScale(-1, 1);
	else
		sprite.setScale(1, 1);

	ResourceManagement::playSound("RobotDie");
}

void Robot::update()
{
	if (is_dead)
	{
		if (death_counter != 0)
			death_counter--;
		else
			should_update = false;	//移除自身
		return;
	}

	robot_walking->setFrame();

	//unsigned char collision; 

	/*if (walk_direcion == right)
		collision = MapCollision::map_collision(coord.x - 5, coord.y - 5);
	else
		collision = MapCollision::map_collision(coord.x - 28, coord.y - 5);*/

	unsigned char collision_precise = MapCollision::map_collision(&hit_box);

	if (collision_precise & (1 << 4))
	{
		die();
		return;
	}

	//下落
	if (!(collision_precise & (1 << 3)))
	{
		//std::cout << "fall\n";
		if (vertical_speed < MAX_FALL_SPEED)
			vertical_speed += GRAVITY;
	}
	else
	{
		vertical_speed = reference_system_vertical_speed;

		//std::cout << stand_on_movable_wall << '\n';
		if (stand_on_movable_wall)
			coord.y = movable_wall_up_edge - 25.0;
		else
		{
			float delta = coord.y - 4 - floor(coord.y / CELL_SIZE) * CELL_SIZE;
			if (delta != 0)
				coord.y -= delta - 3;
		}
	}

	// 检查是否撞墙已经前方是否有路
	// 满足条件继续前进
	// 不满足条件就转向
	if (walk_direcion == left)
	{
		if ((edge_check ? !(MapCollision::getCell(coord.x - 28, coord.y + 40) == Wall) : false) || (collision_precise & (1 << 0)))
		{
			sprite.setScale(-1, 1);
      		walk_direcion = right;
			horizontal_speed = GOOMBA_SPEED + reference_system_horizontal_speed;
		}
		else
		{
			horizontal_speed = -GOOMBA_SPEED + reference_system_horizontal_speed;
		}
	}
	else
	{
		if ((edge_check ? !(MapCollision::getCell(coord.x + 28, coord.y + 40) == Wall) : false) || (collision_precise & (1 << 1)))
		{
			sprite.setScale(1, 1);
			walk_direcion = left;
			horizontal_speed = -GOOMBA_SPEED + reference_system_horizontal_speed;
		}
		else
		{
			horizontal_speed = GOOMBA_SPEED + reference_system_horizontal_speed;
		}
	}

	/*unsigned char i_c = MapCollision::map_collision(getHitBox());
	if (i_c & (1 << 3))
	{
		coord.y = static_cast<short>(coord.y + 3) / CELL_SIZE * CELL_SIZE - 3;
	}*/

	this->move();

	stand_on_movable_wall = false;


	checkMailBoxAndReact();
}

const sf::FloatRect* Robot::getHitBox()
{
	this->hit_box.left = coord.x - 24;
	this->hit_box.top = coord.y;
	
	return &this->hit_box;
}

void Robot::setIsDead(bool i_is_dead)
{
	if (is_dead == true && i_is_dead == false)
	{
		robot_walking->setFrame();
	}

	if (is_dead == false && i_is_dead == true)
	{
		initTexture(5);
		sprite.setTextureRect(sf::IntRect(0, 0, 64, 64));
		sf::Vector2f origin(32.0f, 32.0f);
		sprite.setOrigin(origin);
	}

	is_dead = i_is_dead;
}

const unsigned short Robot::encode() const
{
	/*
	从左往右
	第一位：is_dead
	第二位：walk_direction	0代表左，1代表右
	第三位至第六位：动画帧数
	第七位：edge_check
	*/

	unsigned short eigen_value = 0;

	if (is_dead)
		eigen_value += pow(2, 0);

	if (walk_direcion == right)
		eigen_value += pow(2, 1);

	eigen_value += robot_walking->getCurrentFrame() << 2;

	eigen_value += edge_check << 7;

	return eigen_value;
}

void Robot::decode(const unsigned short i_eigen_code)
{
	//切换死亡贴图
	if (!is_dead && i_eigen_code & (1 << 0))
	{
		initTexture(5);
		sprite.setTextureRect(sf::IntRect(0, 0, 64, 64));
		sf::Vector2f origin(32.0f, 32.0f);
		sprite.setOrigin(origin);
	}
	
	if (i_eigen_code & (1 << 0))
		is_dead = true;
	else
	{
		if (is_dead == true)
		{
			sprite.setOrigin(32, 32);
			robot_walking->setFrameForcely((i_eigen_code >> 2) & 15);
		}
		is_dead = false;
	}

	if (i_eigen_code & (1 << 1))
	{
		sprite.setScale(-1, 1);
		walk_direcion = right;
	}
	else
	{
		sprite.setScale(1, 1);
		walk_direcion = left;
	}
	
	//更新行走贴图
	if (!is_dead)
		robot_walking->setCurrentFrameForcely((i_eigen_code >> 2) & 15);

	edge_check = (i_eigen_code >> 7) & 1;
}

void Robot::setWalkDirection(Direction dir)
{
	walk_direcion = dir;
	if (walk_direcion == right)
		sprite.setScale(-1, 1);
}

void Robot::setEdgeCheck(const bool check)
{
	edge_check = check;
}

//处理邮件
void Robot::react()
{
	MessageQueue::Mail mail = check();

	if (mail.from == MessageQueue::blank_code)
		return;

	//筛选出来自蘑菇怪的消息
	if (MessageQueue::computeInterclassicCode(mail.from) == MessageQueue::robot_header)
	{
		if (mail.message == "collision from left")
		{
			sprite.setScale(-1, 1);
			walk_direcion = right;
		}
		else if (mail.message == "collision from right")
		{
			sprite.setScale(1, 1);
			walk_direcion = left;
		}
	}
	//筛选出来自玩家的消息
	else if (mail.from == MessageQueue::player_code)
	{
		if (mail.message == "defeated by player")
		{
			//增加分数
			points += 100;
			die();
		}
	}
}