#include "Spark.h"

Spark::Spark(int x, int y, ObjectManagement* i_object_manager, unsigned int postal_code)
	: Object(x + 32, y + 32)
{
	spark_burning = new Animation(6, 6, getSprite(), 64, 64);

	sprite.setOrigin(32, 32);

	hit_box.width = 48;
	hit_box.height = 52;
	hit_box.top = y + 12;
	hit_box.left = x + 8;

	i_object_manager->pushBackObject(this);


	//��Ҳ����ܵ�ʱ�䵹����Ӱ��
	can_be_affected_by_time_control = false;
	this->postal_code = postal_code;
}

Spark::Spark(int x, int y, ObjectManagement* i_object_manager)
	: Spark::Spark(x, y, i_object_manager, 0)
{
	this->postal_code = MessageQueue::getPostalCode(MessageQueue::spark_header);
}

Spark::~Spark()
{
	delete spark_burning;
}

void Spark::update()
{
	if (should_update == true)
	{
		spark_burning->setFrame();
	}

	checkMailBoxAndReact();
}

const sf::FloatRect* Spark::getHitBox()
{
	return &this->hit_box;
}


const unsigned short Spark::encode() const
{
	//����ʱ�䵹��Ӱ��
	//����Ҫ��������

	return 0U;
}

void Spark::decode(const unsigned short i_eigen_code)
{

}

//�����ʼ�
void Spark::react()
{
	MessageQueue::Mail mail = check();

	if (mail.from == MessageQueue::blank_code)
		return;

	//ɸѡ��������ҵ���Ϣ
	if (mail.from == MessageQueue::player_code)
	{
		if (mail.message == "hitted by player")
		{
			spark_num++;
			this->should_update = false;
			ResourceManagement::playSound("SparkAbsorbed");
		}
	}
}