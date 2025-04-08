#include "Drawbar.h"

Drawbar::Drawbar(int i_x, int i_y, ObjectManagement* i_object_manager)
	: Receptor::Receptor(i_x, i_y, i_object_manager)
{
	MessageQueue::registerInteractiveSubscriber(this->postal_code);

	this->hitted_by_player = false;
	this->f_is_pressed = false;
}

Drawbar::Drawbar(const MapNote::ReceptorAndEffectors& receptor_info, ObjectManagement* i_object_manager)
	: Receptor::Receptor(receptor_info, i_object_manager)
{
	initTexture(10);
	this->sprite.setTextureRect(sf::IntRect{0, 0, CELL_SIZE, CELL_SIZE});

	this->sprite.setOrigin(0, 0);

	this->hit_box.width = CELL_SIZE * 3;
	this->hit_box.height = CELL_SIZE * 2;
	this->hit_box.left = (receptor_info.receptor.x - 1) * CELL_SIZE;
	this->hit_box.top = (receptor_info.receptor.y - 1) * CELL_SIZE;

	this->hitted_by_player = false;
	this->f_is_pressed = false;

	MessageQueue::registerInteractiveSubscriber(this->postal_code);
}

Drawbar::~Drawbar()
{
	MessageQueue::deregisterInteractiveSubscriber(this->postal_code);
}

void Drawbar::update()
{
	prev_state = state;
	checkAllMail();

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

const sf::FloatRect* Drawbar::getHitBox()
{
	return &hit_box;
}

void Drawbar::react()
{
	MessageQueue::Mail mail = check();

	if (mail.from == MessageQueue::blank_code)
		return;

	if (mail.from == MessageQueue::player_code)
	{
		if (mail.message == "hitted by player")
		{
			hitted_by_player = true;
		}
	}
	else if (mail.from == MessageQueue::game_code)
	{
		if (mail.message == "f is pressed")
		{
			f_is_pressed = true;
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