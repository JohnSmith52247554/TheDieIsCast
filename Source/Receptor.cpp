#include "Receptor.h"

Receptor::Receptor(int i_x, int i_y, ObjectManagement* i_object_manager)
	: Object::Object(i_x, i_y)
{
	i_object_manager->pushBackObject(this);

	delay_counter = 0;

	state = ReceptorState::release;
	prev_state = ReceptorState::release;

	postal_code = MessageQueue::getPostalCode(MessageQueue::receptor_header);

	interactivable = true;
}

Receptor::Receptor(const MapNote::ReceptorAndEffectors& receptor_info, ObjectManagement* i_object_manager)
	: Receptor(receptor_info.receptor.x* CELL_SIZE, receptor_info.receptor.y* CELL_SIZE, i_object_manager)
{
	activation_message = receptor_info.receptor.activation_message;
	release_message = receptor_info.receptor.release_message;

	effectors_list = receptor_info.effector_postal_code;

	can_be_affected_by_time_control = receptor_info.receptor.set_can_be_affected_by_time_control;
}

Receptor::~Receptor()
{

}

const unsigned short Receptor::encode() const
{
	return state;
}

void Receptor::decode(const unsigned short i_eigen_code)
{
	consumeMail();

	if (state == release && static_cast<ReceptorState>(i_eigen_code) == activation)
	{
		state = activation;
		this->sprite.setTextureRect(sf::IntRect{ CELL_SIZE, 0, CELL_SIZE, CELL_SIZE });
	}
	else if (state == activation && static_cast<ReceptorState>(i_eigen_code) == release)
	{
		state = release;
		this->sprite.setTextureRect(sf::IntRect{ 0, 0, CELL_SIZE, CELL_SIZE });
	}
}

void Receptor::getStateByPostalCode(unsigned int& i_postal_code) const
{
	if (state != prev_state)
		i_postal_code = postal_code;
	else
		i_postal_code = MessageQueue::blank_code;
}

void Receptor::f_activation()
{
	this->sprite.setTextureRect(sf::IntRect{ CELL_SIZE, 0, CELL_SIZE, CELL_SIZE });
	if (activation_message != "")
	{
		for (const auto& effector : effectors_list)
		{
			send(effector, activation_message);
		}
	}
	ResourceManagement::playSound("ReceptorActived");
}

void Receptor::f_release()
{
	this->sprite.setTextureRect(sf::IntRect{ 0, 0, CELL_SIZE, CELL_SIZE });
	if (release_message != "")
	{
		for (const auto& effector : effectors_list)
		{
			send(effector, release_message);
		}
	}
	ResourceManagement::playSound("ReceptorActived");
}