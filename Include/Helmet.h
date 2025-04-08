#pragma once

#include <random>
#include <cmath>

#include "Config.h"
#include "Drawbar.h"

class Helmet : public Drawbar
{
private:
	mutable sf::RectangleShape grain_rect;
	struct Grain
	{
		float radius;
		float speed;
		float angular_velocity;	//∂»√ø÷°
		float angle;	//ª°∂»
		float rotation_velocity;
		float rotation_angle;
	};

	const float MAX_GRAIN_RADIUS = CELL_SIZE * 2;
	const float MIN_GRAIN_SPEED = 0.05f;
	const float MAX_GRAIN_SPEED = 0.2f;
	const float MIN_GRAIN_ANGULAR_VELOCITY = 1.f;
	const float MAX_GRAIN_ANGULAR_VELOCITY = 3.f;
	const float MAX_ROTATION_VELOCITY = 0.05f;

	mutable std::array<Grain, HELMET_GRAIN_EFFECT_DENSITY> grain_effect;

public:
	Helmet(int i_x, int i_y , ObjectManagement* i_object_manager);

	virtual void update() override;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	virtual void f_activation() override;
	virtual void f_release() override;

	void initGrain(Grain& grain) const;
};