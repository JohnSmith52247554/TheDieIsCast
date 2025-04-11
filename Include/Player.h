#pragma once

#include "Config.h"
#include "Object.h"
#include "ObjectManagement.h"

class Player : public Object
{
	//变量
private:
	unsigned char jump_coolen;
	bool is_jumping;
	bool is_walking;
	bool continue_walking;
	bool is_falling;
	bool has_failed;
	unsigned char treble_jump;
	unsigned char jump_timer;
	char death_counter;
	bool dialogue_mode;

	bool jump_key_pressed;
	bool walk_left_key_pressed;
	bool walk_right_key_pressed;
#if SHOW_CONSOLE
	bool w_pressed, s_pressed;
#endif

	Animation* player_walking;
	Animation* player_fail;

	const short PLAYER_TEXTURE_WEIDTH = 32;
	const short PLAYER_TEXTURE_HEIGHT = 32;

	unsigned short stand_texture_id, jump_texture_id;
	
	//函数
public:
	//构造析构
	Player(int x, int y, ObjectManagement* object_manager, bool with_helmet);
	virtual ~Player() override;

	//更新
	virtual void update() override;

	//运动
	virtual const sf::FloatRect* getHitBox() override;

	//传出
	const bool getHasFailed() const;

	void stop();
private:
	void jump();
	void failed();

	//处理邮件
	virtual void react() override;

public:
	//特征编码
	//编码
	virtual const unsigned short encode() const override;
	//解码
	virtual void decode(const unsigned short i_eigen_code) override;

	void getStates(float& player_vertical_coord, float& player_horizontal_coord, unsigned short& player_eigen_code) const;
};

