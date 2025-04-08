#include "NPC.h"

NPC::NPC(const MapNote::NPC& NPC_info, ObjectManagement* i_object_manager)
	: Object::Object(NPC_info.x * CELL_SIZE, NPC_info.y * CELL_SIZE)
{
	i_object_manager->pushBackObject(this);

	this->npc_id = NPC_info.npc_id;

	this->postal_code = MessageQueue::getPostalCode(MessageQueue::NPC_header);

	this->hitted_by_player = false;
	this->f_is_pressed = false;

	this->sprite.setTexture(*ResourceManagement::getTexture(NPC_info.npc_skin_id));
	this->sprite.setPosition(coord);
	this->sprite.setOrigin(sprite.getTextureRect().width / 2, sprite.getTextureRect().height / 2);

	float bottom = sprite.getGlobalBounds().top + sprite.getGlobalBounds().height;
	float delta = ceil(bottom / CELL_SIZE) * CELL_SIZE - bottom;
	if (delta != 0)
	{
		coord.y += delta;
		this->sprite.setPosition(coord);
	}

	this->hit_box.width = CELL_SIZE * 7;
	this->hit_box.height = CELL_SIZE * 7;

	this->delay_counter = 60;

	this->name_box.setFont(*ResourceManagement::getFont(0));
	this->name_box.setString(NPC_info.name);
	this->name_box.setCharacterSize(48);
	this->name_box.setScale(0.25, 0.25);
	this->name_box.setFillColor(sf::Color::White);
	this->name_box.setPosition(
		sprite.getPosition().x - name_box.getGlobalBounds().width / 2,
		sprite.getPosition().y - sprite.getGlobalBounds().height / 2 - name_box.getGlobalBounds().height - 15
		);

	MessageQueue::registerInteractiveSubscriber(this->postal_code);

	this->interactivable = true;

	this->can_be_affected_by_time_control = false;

	this->is_dialing = false;
}

NPC::~NPC()
{
	MessageQueue::deregisterInteractiveSubscriber(this->postal_code);
}

void NPC::update()
{
	checkAllMail();

	if (delay_counter != 0)
	{
		delay_counter--;
		if (delay_counter == 0)
			send_message = true;
		return;
	}

	if (hitted_by_player && f_is_pressed)
	{
		std::stringstream text;
		text << "dialogue begin, npc id " << npc_id;
		send(MessageQueue::scene_code, text.str());
		this->send_message = false;
		this->delay_counter = 60;
		this->is_dialing = true;
	}

	if (hitted_by_player == false && is_dialing)
	{
		send(MessageQueue::scene_code, "end npc dialogue");
		is_dialing = false;
	}

	hitted_by_player = false;
	f_is_pressed = false;
}

const sf::FloatRect* NPC::getHitBox()
{
	this->hit_box.left = coord.x - CELL_SIZE * 3.5;
	this->hit_box.top = coord.y - CELL_SIZE * 3.5;

	return &hit_box;
}

const unsigned short NPC::encode() const
{
	return 0;
}

void NPC::decode(const unsigned short i_eigen_code)
{

}

void NPC::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(sprite, states);
	target.draw(name_box, states);
}

const bool NPC::getIsDialing() const
{
	return is_dialing;
}

void NPC::react()
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
			f_is_pressed = true;
	}
	else if (mail.from == MessageQueue::scene_code)
	{
		if (mail.message == "dialogue end")
			is_dialing = false;
	}
}