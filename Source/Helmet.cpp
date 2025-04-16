#include "Helmet.h"
#include "MessageQueueSubscriber.h"

Helmet::Helmet(int i_x, int i_y, ObjectManagement* i_object_manager)
	: Drawbar::Drawbar(i_x - CELL_SIZE / 2, i_y + CELL_SIZE / 2, i_object_manager)
{
	initTexture(14);

	this->hit_box.width = CELL_SIZE * 5;
	this->hit_box.height = CELL_SIZE * 5;
	this->hit_box.left = coord.x - CELL_SIZE * 2.5;
	this->hit_box.top = coord.y - CELL_SIZE * 2.5;

	this->activation_message = "helmet on";
	this->release_message = "";
	this->effectors_list.push_back(2 << 26);

	this->can_be_affected_by_time_control = false;

	grain_rect.setFillColor(sf::Color(202, 217, 246));
	grain_rect.setSize({ 4.f, 4.f });

	for (auto& grain : grain_effect)
	{
		initGrain(grain);
	}
}

void Helmet::update()
{
	checkAllMail();

	for (auto& grain : grain_effect)
	{
		if (grain.radius >= MAX_GRAIN_RADIUS)
			initGrain(grain);
		grain.radius += grain.speed;
		grain.angle += grain.rotation_velocity / grain.radius;
		grain.rotation_angle += grain.angular_velocity;
	}

	if (delay_counter != 0)
	{
		delay_counter--;
		return;
	}

	if (hitted_by_player && f_is_pressed)
	{
		delay_counter = RECEPTOR_DELAY;
		if (state == ReceptorState::release)
		{
			state = ReceptorState::activation;
			f_activation();
		}
		else
		{
			state = ReceptorState::release;
			f_release();
		}
	}

	this->hitted_by_player = false;
	this->f_is_pressed = false;
}

void Helmet::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	for (auto& grain : grain_effect)
	{
		grain_rect.setPosition(coord.x + grain.radius * std::cos(grain.angle), coord.y + grain.radius * std::sin(grain.angle));
		float distance_factor = 1- grain.radius / MAX_GRAIN_RADIUS;
		grain_rect.setScale(0.1 + distance_factor * 0.9, 0.1 + distance_factor * 0.9);
		grain_rect.setRotation(grain.rotation_angle);
		target.draw(grain_rect);
	}

	target.draw(sprite, states);
}

void Helmet::f_activation()
{
	if (activation_message != "")
	{
		for (const auto& effector : effectors_list)
		{
			send(effector, activation_message);
		}
	}
}

void Helmet::f_release()
{
	if (release_message != "")
	{
		for (const auto& effector : effectors_list)
		{
			send(effector, release_message);
		}
	}
}

void Helmet::initGrain(Helmet::Grain& grain) const
{
	std::uniform_real_distribution<> random_angle(0, 2 * 3.1415926);
	std::uniform_real_distribution<> random_radius(0, MAX_GRAIN_RADIUS / 3);
	std::uniform_real_distribution<> random_speed(MIN_GRAIN_SPEED, MAX_GRAIN_SPEED);
	std::uniform_real_distribution<> random_angular_velocity(MIN_GRAIN_ANGULAR_VELOCITY, MAX_GRAIN_ANGULAR_VELOCITY);
	std::uniform_real_distribution<> random_rotation_velocity(-MAX_ROTATION_VELOCITY, MAX_ROTATION_VELOCITY);
	grain.angle = random_angle(gen);
	grain.radius = random_radius(gen);
	grain.speed = random_speed(gen);
	grain.angular_velocity = random_angular_velocity(gen);
	grain.rotation_velocity = random_rotation_velocity(gen);
	grain.rotation_angle = 0.f;
}