#pragma once

#include "Config.h"
#include "MapCollision.h"
#include "Animation.h"
#include "ResourceManagement.h"
#include "MessageQueueSubscriber.h"

class Object : public MessageQueueSubscriber, public sf::Drawable, public sf::Transformable
{
	//变量
protected:
	mutable sf::Sprite sprite;
	sf::Vector2f coord;

	float horizontal_speed;
	float vertical_speed;

	float reference_system_horizontal_speed;
	float reference_system_vertical_speed;

	sf::FloatRect hit_box;

	bool can_be_affected_by_time_control;

	//控制加载
	bool should_update;

	bool interactivable;

	bool stand_on_movable_wall;
	float movable_wall_up_edge;

	//函数
public:
	//构造析构
	Object();
	Object(int x, int y);
	Object(int x, int y, unsigned int i_postal_code);
	virtual ~Object() {};

	void setPosition(int x, int y);

	void setCanBeAffectedByTimeControl(bool cbabtc);

	void setReferenceSystem(float i_horizontal_speed, float i_vertical_speed);
	void setReferenceSystem(float i_horizontal_speed, float i_vertical_speed, bool on_the_wall, float wall_up_edge);
	
	virtual void setState(sf::Vector2f position, float i_horizontal_speed, float i_vertical_speed);

	//初始化
	void initTexture(int ID);

	//传出
	sf::Sprite* getSprite();
	const sf::Vector2f getCoord() const;
	float getVerticalSpeed() const;
	float getHorizontalSpeed() const;
	const bool canBeAffectedByTimeControl() const;
	const bool getShouldUpdate() const;
	const bool getInteractivable() const;

	//更新
	virtual void update() = 0;

	//运动
protected:
	void move();

public:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const override;

	virtual const sf::FloatRect* getHitBox() = 0;

	/*
	特征编码
	使用一个16位二进制数
	实现实体状态的高密度储存和快速读取
	不同实体有着不同的编码和解码规则
	*/
	//编码
	virtual const unsigned short encode() const = 0;
	//解码
	virtual void decode(const unsigned short i_eigen_code) = 0;

	virtual const unsigned short saveCode();
	virtual void praseSaveCode(const unsigned short save_code);

};

