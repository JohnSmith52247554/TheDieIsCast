#pragma once

#include "Config.h"
#include "Object.h"
#include "ObjectManagement.h"
#include "Debug.h"

#include <functional>

class Shadow : public Object
{
private:
	using recodeFunction = std::function<void(float&, float&, unsigned short&, unsigned int&)>;
	recodeFunction get_record;

public:
	Shadow(int x, int y, ObjectManagement* object_manager, recodeFunction function);
	~Shadow();

	virtual void update() override;

	virtual const unsigned short encode() const override;
	virtual void decode(const unsigned short i_eigen_code) override;

	virtual const sf::FloatRect* getHitBox() override;

	//virtual void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const override;

private:
	virtual void react() override;
}; 
