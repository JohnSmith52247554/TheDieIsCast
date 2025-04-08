#pragma once

#include "Config.h"
#include "Object.h"
#include "Animation.h"
#include "ObjectManagement.h"

class Spark : public Object 
{
	//变量
private:
	Animation* spark_burning;

	//函数
public:
	Spark(int x, int y, ObjectManagement* i_object_manager, unsigned int postal_code);
	Spark(int x, int y, ObjectManagement* i_object_manager);
	virtual ~Spark() override;

	virtual void update() override;

	const sf::FloatRect* getHitBox() override;

	//编码
	virtual const unsigned short encode() const override;
	//解码
	virtual void decode(const unsigned short i_eigen_code) override;

private:
	//处理邮件
	virtual void react() override;
};

