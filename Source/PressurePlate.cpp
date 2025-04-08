#include "PressurePlate.h"

PressurePlate::PressurePlate(const MapNote::ReceptorAndEffectors& receptor_info, ObjectManagement* i_object_manager)
	: Receptor::Receptor(receptor_info, i_object_manager)
{
	initTexture(11);
	this->sprite.setTextureRect(sf::IntRect{ 0, 0, CELL_SIZE, CELL_SIZE });

	this->sprite.setOrigin(0, 0);

	this->hit_box.width = CELL_SIZE;
	this->hit_box.height = CELL_SIZE * 0.3;
	this->hit_box.left = coord.x;
	this->hit_box.top = coord.y + 0.7 * CELL_SIZE;

	this->interactivable = false;
}

PressurePlate::~PressurePlate()
{

}

void PressurePlate::update()
{
	prev_state = state;
	if (delay_counter != 0)
	{
		delay_counter--;
		return;
	}

	if (this->remained_mail_counter == 0 && state == activation)
	{
		state = ReceptorState::release;
		f_release();
		delay_counter = RECEPTOR_DELAY;
	}
	checkMailBoxAndReact();
}

const sf::FloatRect* PressurePlate::getHitBox()
{
	return &hit_box;
}

void PressurePlate::react()
{
	MessageQueue::Mail mail = check();

	if (mail.from == MessageQueue::blank_code)
	{
		if (state == activation)
		{
			state = ReceptorState::release;
			f_release();
			delay_counter = RECEPTOR_DELAY;
		}
		return;
	}

	if (mail.from == MessageQueue::player_code)
	{
		if (mail.message == "hitted by player")
		{
			if (state == release)
			{
				state = activation;
				delay_counter = RECEPTOR_DELAY;
				f_activation();
			}
		}
	}
	else if (MessageQueue::computeInterclassicCode(mail.from) == MessageQueue::robot_header)
	{
		if (mail.message == "hitted by enemy")
		{
			if (state == release)
			{
				state = activation;
				delay_counter = RECEPTOR_DELAY;
				f_activation();
			}
		}
	}
	else if (MessageQueue::computeInterclassicCode(mail.from) == MessageQueue::shadow_header)
	{
		if (mail.message == "force")
		{
			if (state == activation)
			{
				state = ReceptorState::release;
				f_release();
				delay_counter = RECEPTOR_DELAY;
			}
			else
			{
				state = activation;
				delay_counter = RECEPTOR_DELAY;
				f_activation();
			}
		}
	}
}