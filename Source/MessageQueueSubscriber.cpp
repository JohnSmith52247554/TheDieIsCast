#include "MessageQueueSubscriber.h"

std::array<unsigned int, 12> MessageQueue::postal_code_counter;
std::vector<MessageQueueSubscriber*> MessageQueue::subscribers;
std::deque<MessageQueue::Mail> MessageQueue::mail_box;
std::vector<unsigned int> MessageQueue::interactivable_subscriber;

const MessageQueue::Mail MessageQueue::blank_mail =
{
	MessageQueue::blank_code, MessageQueue::blank_code, ""
};


void MessageQueue::resetPostalCodeCounter()
{
	mail_box.clear();

	for (auto& i : postal_code_counter)
	{
		i = 0;
	}
}

unsigned int MessageQueue::getIntraclassicCode(const unsigned char interclassic_code)
{
	switch (interclassic_code)
	{
	case interface_header:	//界面
		return postal_code_counter.at(0)++;
	case robot_header:	//蘑菇怪
		return postal_code_counter.at(1)++;
	case spark_header:	//金币
		return postal_code_counter.at(2)++;
	case dialogue_interface_header:	//对话界面
		return postal_code_counter.at(3)++;
	case movable_wall_header:		//可移动墙壁
		return postal_code_counter.at(4)++;
	case NPC_header:	//NPC
		return postal_code_counter.at(5)++;
	case receptor_header:	//所有的感受器
		return postal_code_counter.at(6)++;
	case laser_header:
		return postal_code_counter.at(7)++;
	case laser_gun_header:
		return postal_code_counter.at(8)++;
	case robot_factory_header:
		return postal_code_counter.at(9)++;
	case shadow_generator_header:
		return postal_code_counter.at(10)++;
	case shadow_header:
		return postal_code_counter.at(11)++;
	default:
		break;
	}
	return 0;
}

unsigned int MessageQueue::getPostalCode(const unsigned char interclassic_code)
{
	return (interclassic_code << 26) + getIntraclassicCode(interclassic_code);
}

unsigned char MessageQueue::computeInterclassicCode(const unsigned int full_postal_code)
{
	return static_cast<unsigned char>(full_postal_code >> 26);
}

void MessageQueue::send(unsigned int from, unsigned int to, std::string message)
{
	Mail mail =
	{
		from,
		to,
		message
	};
	send(mail);
}

void MessageQueue::send(Mail mail)
{
	if (mail.to == all_interactivable_code)
	{
		for (auto& code : interactivable_subscriber)
		{
			send(mail.from, code, mail.message);
		}
	}
	else
		mail_box.push_back(mail);

	if (mail_box.size() > MESSAGE_QUEUE_CAPACITY)
	{
		mail_box.pop_front();
	}

	//std::cout << mail_box.size() << '\n';

	//设置counter
	for (auto& subscriber : subscribers)
	{
		if (subscriber->getPostalCode() == mail.to)
		{
			subscriber->increaseCounter();
			return;
		}
	}
}

MessageQueue::Mail MessageQueue::checkMailBox(unsigned int to_code)
{
	for (auto mail = mail_box.begin(); mail != mail_box.end(); mail++)
	{
		if ((*mail).to == to_code)
		{
			Mail mail_copy = *mail;
			mail_box.erase(mail);
			return mail_copy;
		}
	}

	return blank_mail;
}

MessageQueueSubscriber::MessageQueueSubscriber()
{
	send_message = true;
	MessageQueue::registerSubscriber(this);
	postal_code = MessageQueue::blank_code;
	remained_mail_counter = 0;
}

MessageQueueSubscriber::~MessageQueueSubscriber()
{
	MessageQueue::deregisterSubscriber(this);
}

const bool MessageQueueSubscriber::getSendMessage() const
{
	return send_message;
}

const unsigned int MessageQueueSubscriber::getPostalCode() const
{
	return postal_code;
}

void MessageQueueSubscriber::send(unsigned int to, std::string message)
{
	MessageQueue::send(postal_code, to, message);
}

MessageQueue::Mail MessageQueueSubscriber::check()
{
	MessageQueue::Mail mail = MessageQueue::checkMailBox(postal_code);
	remained_mail_counter--;
	return mail;
}

void MessageQueue::registerSubscriber(MessageQueueSubscriber* subscriber)
{
	subscribers.push_back(subscriber);
}

void MessageQueue::deregisterSubscriber(MessageQueueSubscriber* i_subscriber)
{
	/*subscribers.erase(std::remove_if(subscribers.begin(), subscribers.end(), [=](MessageQueueSubscriber* ptr) {return ptr = i_subscriber; }),
		subscribers.end());*/
	for (auto subscriber = subscribers.begin(); subscriber != subscribers.end(); subscriber++)
	{
		if (*subscriber == i_subscriber)
		{
			subscribers.erase(subscriber);
			return;
		}
	}
}

void MessageQueue::registerInteractiveSubscriber(unsigned int subscriber)
{
	interactivable_subscriber.push_back(subscriber);
}

void MessageQueue::deregisterInteractiveSubscriber(unsigned int i_subscriber)
{
	/*interactivable_subscriber.erase(std::remove_if(interactivable_subscriber.begin(), interactivable_subscriber.end(), [&](unsigned short& check) {return check = i_subscriber; }),
		interactivable_subscriber.end());*/
	for (auto subscriber = interactivable_subscriber.begin(); subscriber != interactivable_subscriber.end(); subscriber++)
	{
		if (*subscriber == i_subscriber)
		{
			interactivable_subscriber.erase(subscriber);
			return;
		}
	}
}

const short MessageQueue::getMailBoxSize()
{
	return mail_box.size();
}

void MessageQueue::savePostalCodeCounter(std::ofstream& file)
{
	for (auto& counter : postal_code_counter)
		file.write(reinterpret_cast<char*>(&counter), sizeof(counter));
}
void MessageQueue::loadPostalCodeCounter(std::ifstream& file)
{
	for (int i = 0; i < postal_code_counter.size(); i++)
	{
		// 跳过地图注释中的部分，防止编码错误
		if (i == 4 || i == 5 || i == 6)
		{
			file.seekg(sizeof(postal_code_counter.at(0)), std::ios::cur);
			continue;
		}
		file.read(reinterpret_cast<char*>(&postal_code_counter.at(i)), sizeof(postal_code_counter.at(i)));
	}
}

void MessageQueue::clear()
{
	mail_box.clear();
}

void MessageQueueSubscriber::increaseCounter()
{
	remained_mail_counter++;
}

void MessageQueueSubscriber::checkMailBoxAndReact()
{
	if (static_cast<unsigned short>(remained_mail_counter) > 0U)
	{
		react();
	}
}

void MessageQueueSubscriber::checkAllMail()
{
	while (remained_mail_counter > 0)
	{
		react();
	}
}

void MessageQueueSubscriber::consumeMail()
{
	while (remained_mail_counter > 0)
	{
		check();
	}
}

void MessageQueueSubscriber::react()
{
	MessageQueue::Mail mail = check();
}