#pragma once

#include "Config.h"
#include "Object.h"
#include "MapNote.h"
#include "ObjectManagement.h"

class NPC : public Object
{
	//变量
private:
	short npc_id;
	short delay_counter;

	bool hitted_by_player, f_is_pressed;

	sf::Text name_box;	//在头顶上方显示名字

	bool is_dialing;

	//函数
public:
	NPC(const MapNote::NPC& NPC_info, ObjectManagement* i_object_manager);
	virtual ~NPC() override;

	//更新
	virtual void update() override;

	//运动
	virtual const sf::FloatRect* getHitBox() override;

	//编码
	virtual const unsigned short encode() const override;
	//解码
	virtual void decode(const unsigned short i_eigen_code) override;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const override;

	const bool getIsDialing() const;

private:
	//处理邮件
	virtual void react() override;
};