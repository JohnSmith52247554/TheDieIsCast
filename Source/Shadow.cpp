#include "Shadow.h"

Shadow::Shadow(int x, int y, ObjectManagement* object_manager, recodeFunction function)
	: Object(x, y)
{
	get_record = function;

	object_manager->pushBackObject(this);
	initTexture(17);

	hit_box.height = 32;
	hit_box.width = 32;

	postal_code = MessageQueue::getPostalCode(MessageQueue::shadow_header);

	can_be_affected_by_time_control = false;

	/*auto shader_path = EXE_DIR / "Resources" / "Shader" / "noise.frag";
	if (!noise_shader.loadFromFile(shader_path.string(), sf::Shader::Fragment))
		std::cerr << "Failed to load shader" << std::endl;*/
}

Shadow::~Shadow()
{
	std::cout << "delete\n";
}

void Shadow::update()
{
	/*if (*list_ptr >= *list_size)
	{
		should_update = false;
		return;
	}*/

	checkMailBoxAndReact();
	if (should_update == false)
		return;

	unsigned short player_eigen_code;
	unsigned int interact_postal_code;

	float new_x;
	get_record(coord.y, new_x, player_eigen_code, interact_postal_code);
	if (new_x > coord.x)
		sprite.setScale(1, 1);
	else
		sprite.setScale(-1, 1);
	coord.x = new_x;

	if (interact_postal_code != MessageQueue::blank_code)
	{
		send(interact_postal_code, "force");
	}

	//(*list_ptr)++;
}

const unsigned short Shadow::encode() const
{
	return 0U;
}

void Shadow::decode(const unsigned short i_eigen_code)
{
	/*unsigned short player_eigen_code;
	unsigned int interact_postal_code;

	float new_x;
	get_record(coord.y, new_x, player_eigen_code, interact_postal_code);
	if (new_x > coord.x)
		sprite.setScale(-1, 1);
	else
		sprite.setScale(1, 1);
	coord.x = new_x;*/

	/*if (interact_postal_code != MessageQueue::blank_code)
	{
		send(interact_postal_code, "force");
	}*/
}

const sf::FloatRect* Shadow::getHitBox()
{
	hit_box.left = coord.x - 16;
	hit_box.top = coord.y - 16;

	return &hit_box;
}

//void Shadow::draw(sf::RenderTarget& target, sf::RenderStates states) const
//{
//	states.shader = &noise_shader;
//	target.draw(this->sprite, states);
//}

void Shadow::react()
{
	MessageQueue::Mail mail = check();

	if (mail.from == MessageQueue::blank_code)
		return;

	if (MessageQueue::computeInterclassicCode(mail.from) == MessageQueue::shadow_generator_header)
	{
		if (mail.message == "vanish")
		{
			should_update = false;
		}
	}
}