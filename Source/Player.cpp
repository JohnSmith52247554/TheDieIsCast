#include "Player.h"

Player::Player(int x, int y, ObjectManagement* object_manager, bool with_helmet)
{
	stand_texture_id = with_helmet ? 20 : 0;
	jump_texture_id = with_helmet ? 21 : 1;

	this->coord.x = x;
	this->coord.y = y;
	this->initTexture(stand_texture_id);

	is_jumping = false;
	jump_coolen = 0;
	jump_timer = 0;
	is_walking = false;
	continue_walking = false;
	is_falling = false;
	has_failed = false;
	dialogue_mode = false;
	treble_jump = 0;
	death_counter = DEATH_COUNTER * 3;

	jump_key_pressed = false;
	walk_left_key_pressed = false;
	walk_right_key_pressed = false;

	player_walking = new Animation(with_helmet ? 22 : 2, 4, this->getSprite(), 32, 32);
	player_fail = new Animation(with_helmet ? 23 : 3, 2, this->getSprite(), 32, 32);
	player_fail->setAnimationSpeed(15);

	this->hit_box.width = PLAYER_TEXTURE_WEIDTH;
	this->hit_box.height = PLAYER_TEXTURE_HEIGHT;

	this->postal_code = MessageQueue::player_code;

	object_manager->registerPlayer(this);

#if SHOW_CONSOLE
	w_pressed = false;
	s_pressed = false;
#endif
}

Player::~Player()
{
	delete player_walking;
	delete player_fail;
}

void Player::update()
{
#if SHOW_CONSOLE
	if (cheating_mode)
	{
		if (walk_left_key_pressed)
			horizontal_speed = -PLAYER_SPEED;
		else if (walk_right_key_pressed)
			horizontal_speed = PLAYER_SPEED;
		else
			horizontal_speed = 0;
		if (w_pressed)
			vertical_speed = -PLAYER_SPEED;
		else if (s_pressed)
			vertical_speed = PLAYER_SPEED;
		else
			vertical_speed = 0;

		if (can_be_affected_by_time_control)
			can_be_affected_by_time_control = false;

		has_failed = false;
	}
	else
	{
		if (!can_be_affected_by_time_control)
			can_be_affected_by_time_control = true;
#endif

		//角色死亡相关逻辑
		if (has_failed)
		{
			if (death_counter <= 0)
			{
				if (death_counter < -60)
				{
					return;
				}
				if (death_counter == -60)
				{
					send(MessageQueue::scene_code, "failed");
					death_counter--;
					return;
				}
				if (player_fail->getCurrentFrame() < 1)
					player_fail->setFrame();
			}
			unsigned short collision = MapCollision::map_collision(this->getHitBox());
			if (!(collision & (1 << 3)))
			{
				is_falling = true;
				if (vertical_speed < MAX_FALL_SPEED)
					this->vertical_speed += GRAVITY;
			}
			else
			{
				this->vertical_speed = reference_system_vertical_speed;

				if (!stand_on_movable_wall)
					coord.y = static_cast<short>(coord.y - CELL_SIZE / 2) / CELL_SIZE * CELL_SIZE + CELL_SIZE / 2;
				else
					coord.y = movable_wall_up_edge - CELL_SIZE / 2;
			}

			if (horizontal_speed > reference_system_horizontal_speed)
			{
				if (collision & (1 << 1))
					horizontal_speed = 0;
				this->horizontal_speed -= RESISTANCE;
				if (this->horizontal_speed < reference_system_horizontal_speed)
					this->horizontal_speed = reference_system_horizontal_speed;
			}
			else if (horizontal_speed < reference_system_horizontal_speed)
			{
				if (collision & (1 << 0))
					horizontal_speed = 0;
				this->horizontal_speed += RESISTANCE;
				if (this->horizontal_speed > reference_system_horizontal_speed)
					this->horizontal_speed = reference_system_horizontal_speed;
			}

			death_counter--;
			move();
			stand_on_movable_wall = false;
			return;
		}

		//行走动画逻辑
		if (is_walking && !is_falling)
			continue_walking = true;
		else
		{
			continue_walking = false;
			player_walking->reset();
		}

		//更新贴图
		if (is_falling)
		{
			this->initTexture(jump_texture_id);

			if (horizontal_speed >= -0.01 + reference_system_horizontal_speed)
				this->sprite.setScale(1, 1);
			else
				this->sprite.setScale(-1, 1);
		}
		else if (continue_walking)
		{
			player_walking->setFrame();
			if (walk_right_key_pressed)
				this->sprite.setScale(1, 1);
			else
				this->sprite.setScale(-1, 1);
		}
		else
		{
			this->initTexture(stand_texture_id);
			this->sprite.setScale(1, 1);
		}

		unsigned short collision = MapCollision::map_collision(this->getHitBox());
		//std::cout << (collision & (1 << 0)) << (collision & (1 << 1)) << (collision & (1 << 2)) << (collision & (1 << 3)) << '\n';

		//强制对齐
		bool adjusted = false;
		float cell_x, cell_y;
		cell_y = coord.y;
		cell_x = coord.x - CELL_SIZE / 2;
		if (MapCollision::getCell(cell_x, cell_y) == Wall || MapCollision::getCell(cell_x, cell_y) == Wall)
		{
			coord.x = cell_x + CELL_SIZE / 2 + 1;
			horizontal_speed = 0;
			adjusted = true;
		}
		cell_x = coord.x + CELL_SIZE / 2;
		if (MapCollision::getCell(cell_x, cell_y) == Wall || MapCollision::getCell(cell_x, cell_y) == Wall)
		{
			coord.x = cell_x - CELL_SIZE / 2 - 1;
			horizontal_speed = 0;
			adjusted = true;
		}

		//检测地刺
		if (collision & (1 << 4))
		{
			failed();
			this->sprite.setPosition(coord);
			return;
		}

		bool auto_jump = false;
		//水平移动
		if (dialogue_mode == false)	//剧情模式下无法控制
		{
			if (walk_left_key_pressed)
			{
				//保证不被墙壁阻挡
				cell_x = coord.x - CELL_SIZE / 2 - 2;
				if (!(collision & (1 << 0)) && !(MapCollision::getCell(cell_x, cell_y) == Wall) && !(MapCollision::getCell(cell_x, cell_y) == Wall))
				{
					if (horizontal_speed > -PLAYER_SPEED + reference_system_horizontal_speed)
					{
						if (adjusted == false)
							this->horizontal_speed += -PLAYER_ACCELERATION;
						is_walking = true;
					}
				}
				else if (MapCollision::getCell(cell_x, cell_y - CELL_SIZE) != Wall && MapCollision::getCell(coord.x, coord.y - CELL_SIZE) != Wall)
				{
					vertical_speed += PLAYER_AUTO_JUMP_SPEED;
					auto_jump = true;
				}

			}
			else if (walk_right_key_pressed)
			{
				cell_x = coord.x + CELL_SIZE / 2 + 1;
				if (!(collision & (1 << 1)) && !(MapCollision::getCell(cell_x, cell_y) == Wall) && !(MapCollision::getCell(cell_x, cell_y) == Wall))
				{
					if (horizontal_speed < PLAYER_SPEED + reference_system_horizontal_speed)
					{
						if (adjusted == false)
							this->horizontal_speed += PLAYER_ACCELERATION;
						is_walking = true;
					}
				}
				else if (MapCollision::getCell(cell_x, cell_y - CELL_SIZE) != Wall && MapCollision::getCell(coord.x, coord.y - CELL_SIZE) != Wall)
				{
					vertical_speed += PLAYER_AUTO_JUMP_SPEED;
					auto_jump = true;
				}
			}
			else
			{
				if (horizontal_speed > reference_system_horizontal_speed)
				{
					if (collision & (1 << 1))
						horizontal_speed = 0;
					this->horizontal_speed -= RESISTANCE;
					if (this->horizontal_speed < reference_system_horizontal_speed)
						this->horizontal_speed = reference_system_horizontal_speed;
				}
				else if (horizontal_speed < reference_system_horizontal_speed)
				{
					if (collision & (1 << 0))
						horizontal_speed = 0;
					this->horizontal_speed += RESISTANCE;
					if (this->horizontal_speed > reference_system_horizontal_speed)
						this->horizontal_speed = reference_system_horizontal_speed;
				}
				is_walking = false;
			}
		}
		else
		{
			if (horizontal_speed > reference_system_horizontal_speed)
			{
				this->horizontal_speed -= RESISTANCE;
				if (this->horizontal_speed < reference_system_horizontal_speed)
					this->horizontal_speed = reference_system_horizontal_speed;
			}
			else if (horizontal_speed < reference_system_horizontal_speed)
			{
				this->horizontal_speed += RESISTANCE;
				if (this->horizontal_speed > reference_system_horizontal_speed)
					this->horizontal_speed = reference_system_horizontal_speed;
			}
			is_walking = false;
		}

		if (reference_system_horizontal_speed != 0)
		{
			if ((collision & (1 << 1) && horizontal_speed > 0) || collision & (1 << 0) && horizontal_speed < 0)
			{
				horizontal_speed = 0;
				reference_system_horizontal_speed = 0;
			}
		}

		//检测角色是否悬空
		if (!(collision & (1 << 3)))
		{
			is_falling = true;
			if (vertical_speed < MAX_FALL_SPEED)
				this->vertical_speed += GRAVITY;
			if (dialogue_mode == false)	//剧情模式下无法控制
			{
				if (jump_key_pressed == true && treble_jump < MAX_CONTINUE_JUMP_NUM - 1 && jump_timer == 0 && is_jumping)
				{
					jump();
					treble_jump++;
					jump_timer = JUMP_TIMER;
				}
				if (is_jumping && jump_timer > 0)
					jump_timer--;
			}
		}
		else
		{
			is_falling = false;
			if (is_jumping)
			{
				ResourceManagement::playSound("PlayerLand");
				is_jumping = false;
				jump_coolen = 20;
				jump_timer = 0;
			}
			if (!auto_jump)
				this->vertical_speed = reference_system_vertical_speed;

			if (!stand_on_movable_wall)
				coord.y = static_cast<short>(coord.y - CELL_SIZE / 2) / CELL_SIZE * CELL_SIZE + CELL_SIZE / 2;
			else
				coord.y = movable_wall_up_edge - CELL_SIZE / 2;

			if (dialogue_mode == false)	//剧情模式下无法控制
			{
				if (jump_key_pressed == true && jump_coolen == 0)
				{
					jump();
					jump_timer = JUMP_TIMER;
					treble_jump = 0;
				}
			}
		}

		if (jump_coolen > 0)
			jump_coolen--;

		//头顶撞墙
		if (vertical_speed < 0 && collision & (1 << 2) && !auto_jump)
		{
			this->vertical_speed = 0.01;
		}

		stand_on_movable_wall = false;

#if SHOW_CONSOLE
	}
#endif
	this->move();

	checkAllMail();
}

const sf::FloatRect* Player::getHitBox()
{
	this->hit_box.left = coord.x - CELL_SIZE / 2;
	this->hit_box.top = coord.y - CELL_SIZE / 2;

	return &this->hit_box;
}

const bool Player::getHasFailed() const
{
	return has_failed;
}

void Player::stop()
{
	//std::cout << "stop\n";
	jump_key_pressed = false;
	walk_left_key_pressed = false;
	walk_right_key_pressed = false;
#if SHOW_CONSOLE
	w_pressed = false;
	s_pressed = false;
#endif
}

void Player::jump()
{
	vertical_speed = PLAYER_JUMP_SPEED;
	is_jumping = true;

	//播放音效
	ResourceManagement::playSound("PlayerJump");
}

void Player::failed()
{
	//剧情模式下无敌
	if (dialogue_mode == true)
		return;

	jump_key_pressed = false;
	walk_left_key_pressed = false;
	walk_right_key_pressed = false;

	has_failed = true;
	death_counter = DEATH_COUNTER * 3;

	/*initTexture(3);

	sprite.setTextureRect(sf::IntRect(0, 0, 16, 16));*/
	player_fail->reset();
	player_fail->setFrame();

	//horizontal_speed = 0;
	//vertical_speed = PLAYER_JUMP_SPEED;

	ResourceManagement::playSound("PlayerFailed");
	ResourceManagement::pauseBackgroundMusic();
}

//处理邮件
void Player::react()
{
	MessageQueue::Mail mail = check();

	if (mail.from == MessageQueue::blank_code)
		return;

	//筛选出来自场景的消息
#if SHOW_CONSOLE
	if (mail.from == MessageQueue::game_code && (cheating_mode || dialogue_mode == false))
#else
	if (mail.from == MessageQueue::game_code && dialogue_mode == false)
#endif
	{
		if (mail.message == "up is pressed" || mail.message == "space is pressed")
		{
			jump_key_pressed = true;
		}
		else if (mail.message == "a is pressed" || mail.message == "left is pressed")
		{
			walk_left_key_pressed = true;
		}
		else if (mail.message == "d is pressed" || mail.message == "right is pressed")
		{
			walk_right_key_pressed = true;
		}
		else if (mail.message == "left walking key released")
		{
			walk_left_key_pressed = false;
		}
		else if (mail.message == "right walking key released")
		{
			walk_right_key_pressed = false;
		}
		else if (mail.message == "jump key released")
		{
			jump_key_pressed = false;
		}
#if SHOW_CONSOLE
		else if (mail.message == "w is pressed")
			w_pressed = true;
		else if (mail.message == "w is released")
			w_pressed = false;
		else if (mail.message == "s is pressed")
			s_pressed = true;
		else if (mail.message == "s is released")
			s_pressed = false;
#endif

	}
	else if (mail.from == MessageQueue::scene_code)
	{
		if (mail.message == "dialogue mode begin")
		{
			dialogue_mode = true;
			stop();
		}
		else if (mail.message == "dialogue mode end")
		{
			dialogue_mode = false;
		}
		else if (mail.message == "stop all motion")
		{
			stop();
		}
	}
	//筛选出来自蘑菇怪的消息
	else if (MessageQueue::computeInterclassicCode(mail.from) == MessageQueue::robot_header)
	{
		if (mail.message == "player bounce")
		{
			vertical_speed = PLAYER_BOUNCE_SPEED;
			move();
		}
		else if (mail.message == "player hitted by robot")
		{
#if SHOW_CONSOLE
			if (cheating_mode)
				return;
#endif
			
			failed();
		}
	}
}

const unsigned short Player::encode() const
{
	/*
	从左往右数
	第一位：has_failed
	第二位：is_falling	决定贴图
	第三位：continue_walking	决定贴图
	第四位：is_jumping	跳跃相关逻辑
	第五位和第六位：treble_jump	共有0，1，2三种状态
	第七至第九位：贴图的当前帧
	*/

	unsigned short eigen_code = 0;

	if (has_failed)
		eigen_code += pow(2, 0);

	if (is_falling)
		eigen_code += pow(2, 1);

	if (continue_walking)
		eigen_code += pow(2, 2);

	if (is_jumping)
		eigen_code += pow(2, 3);

	if (treble_jump == 1)
		eigen_code += pow(2, 4);
	else if (treble_jump == 2)
		eigen_code += pow(2, 5);

	if (continue_walking)
		eigen_code += (player_walking->getCurrentFrame() << 6);
	else if (is_jumping)
		eigen_code += 0 << 6;
	else if (has_failed)
	{
		eigen_code += (player_fail->getCurrentFrame()) << 6;
		eigen_code += death_counter << 8;
	}
	else
		eigen_code += 0 << 6;

	return eigen_code;
}

void Player::decode(const unsigned short i_eigen_code)
{
	consumeMail();

	//切换死亡贴图
	if (i_eigen_code & (1 << 0))
	{
		has_failed = true;
		player_fail->setCurrentFrameForcely((i_eigen_code >> 6) & 7);
		death_counter = (i_eigen_code >> 8) & 255;
	}
	else
	{
		if (has_failed == true)
		{
			has_failed = false;
			//send(MessageQueue::scene_code, "play bgm");
		}
	}

	//切换跳跃/掉落贴图
	if (!is_falling && i_eigen_code & (1 << 1))
	{
		this->initTexture(jump_texture_id);
		sprite.setTextureRect(sf::IntRect(0, 0, PLAYER_TEXTURE_WEIDTH, PLAYER_TEXTURE_HEIGHT));
		if (horizontal_speed >= -0.01)
			this->sprite.setScale(1, 1);
		else
			this->sprite.setScale(-1, 1);

		//std::cout << "fall\n";
	}
	if (i_eigen_code & (1 << 1))
		is_falling = true;
	else
		is_falling = false;

	if (i_eigen_code & (1 << 2))
		continue_walking = true;
	else
		continue_walking = false;
	//切换行走动画
	if (!is_falling && continue_walking)
	{
		player_walking->setCurrentFrameForcely((i_eigen_code >> 6) & 7);
		//std::cout << ((i_eigen_code >> 6) & 7) << '\n';
		if (horizontal_speed >= -0.01)
			this->sprite.setScale(1, 1);
		else
			this->sprite.setScale(-1, 1);
	}

	if (!has_failed && !continue_walking && !is_falling)
	{
		initTexture(stand_texture_id);
		sprite.setTextureRect(sf::IntRect(0, 0, PLAYER_TEXTURE_WEIDTH, PLAYER_TEXTURE_HEIGHT));
		//std::cout << "stand\n";
	}

	if (i_eigen_code & (1 << 3))
		is_jumping = true;
	else
		is_jumping = false;

	treble_jump = 0;
	if (i_eigen_code & (1 << 4))
		treble_jump += 1;
	if (i_eigen_code & (1 << 5))
		treble_jump += 2;

}

void Player::getStates(float& player_vertical_coord, float& player_horizontal_coord, unsigned short& player_eigen_code) const
{
	player_vertical_coord = coord.y;
	player_horizontal_coord = coord.x;
	player_eigen_code = this->encode();
}