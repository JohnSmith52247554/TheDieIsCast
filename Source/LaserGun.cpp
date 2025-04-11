#include "LaserGun.h"

LaserGun::LaserGun(int i_x, int i_y, ObjectManagement* i_object_manager, unsigned char direcion_code)
	: LaserGun(i_x, i_y, i_object_manager, direcion_code, 0)
{
	postal_code = MessageQueue::getPostalCode(MessageQueue::laser_gun_header);
}

LaserGun::LaserGun(int i_x, int i_y, ObjectManagement* i_object_manager, unsigned char direcion_code, unsigned int i_postal_code)
	: Object(i_x, i_y)
{
	i_object_manager->pushBackObject(this);
	save_object_manager = i_object_manager;

	shoot_animation = new Animation(12, 12U, &this->sprite, 128, 128);
	shoot_animation->setAnimationSpeed(5);

	hit_box.width = 64;
	hit_box.height = 64;
	hit_box.left = i_x - 32;
	hit_box.top = i_y - 32;

	postal_code = i_postal_code;

	sf::Vector2f center(
		16,
		65
	);
	sprite.setOrigin(center);

	flip_code = direcion_code;

	setFlip(direcion_code);

	can_shoot = false;

	shoot_delay = SHOOT_DELAY;
}

LaserGun::~LaserGun()
{
	if (shoot_animation != nullptr)
		delete shoot_animation;
}

void LaserGun::update()
{
	checkMailBoxAndReact();

	if (shoot_delay != 0 && shoot_animation->getCurrentFrame() == 0)
	{
		shoot_delay--;
		return;
	}

	shoot_animation->setFrame();

	if (shoot_animation->getCurrentFrame() == 1)
	{
		if (can_shoot)
		{
			auto l = new Laser(coord.x, coord.y, save_object_manager, shoot_direction);
			l->setCanBeAffectedByTimeControl(can_be_affected_by_time_control);
			can_shoot = false;
			sf::FloatRect view_port = { view.getCenter().x- view.getSize().x / 2,
				view.getCenter().y - view.getSize().y / 2, view.getSize().x, view.getSize().y };
			if (MapCollision::object_collision(&hit_box, &view_port))
				ResourceManagement::playSound("LaserGunShoot");
		}
	}
	else
	{
		can_shoot = true;
	}
	
	if (shoot_animation->getCurrentFrame() == 11)
	{
		shoot_delay = SHOOT_DELAY;
	}
}

const sf::FloatRect* LaserGun::getHitBox()
{
	return &hit_box;
}

const unsigned short LaserGun::encode() const
{
	/*
	0-2λ��flip_code
	3-6λ��current_frame
	7
	*/
	unsigned short code = flip_code;
	code += shoot_animation->getCurrentFrame() << 3;
	code += can_be_affected_by_time_control << 7;
	return code;
}

void LaserGun::decode(const unsigned short i_eigen_code)
{
	if (flip_code != (i_eigen_code & 7))
	{
		flip_code = (i_eigen_code & 7);
		setFlip(flip_code);
	}

	shoot_animation->setCurrentFrameForcely((i_eigen_code >> 3) & 15);

	can_be_affected_by_time_control = (i_eigen_code >> 7) & 1;
}

void LaserGun::setFlip(unsigned short code)
{
	/*
	����������£�
	fff���������κβ���
	fft�������ҷ�ת����˳ʱ����ת270��
	ftf�����·�ת
	ftt��˳ʱ����ת270��
	tff�����ҷ�ת
	tft��˳ʱ����ת90��
	ttf����ת180�ȣ���ͬ�����·�ת�����ҷ�ת��
	ttt�������ҷ�ת����˳ʱ����ת90��
	*/

	switch (code)
	{
	case 0:
		shoot_direction = right;
		break;
	case 1:
		shoot_direction = down;
		this->sprite.setScale(-1, 1);
		this->sprite.setRotation(-90);
		break;
	case 2:
		shoot_direction = right;
		this->sprite.setScale(1, -1);
		break;
	case 3:
		shoot_direction = up;
		this->sprite.setRotation(-90);
		break;
	case 4:
		shoot_direction = left;
		this->sprite.setScale(-1, 1);
		break;
	case 5:
		shoot_direction = down;
		this->sprite.setRotation(90);
		break;
	case 6:
		shoot_direction = left;
		this->sprite.setRotation(180);
		break;
	case 7:
		shoot_direction = up;
		this->sprite.setScale(-1, 1);
		this->sprite.setRotation(-90);
		break;
	default:
		shoot_direction = right;
		break;
	}
}