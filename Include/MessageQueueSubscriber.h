/*
维护一个消息队列，用于实现不同游戏对象之间的通讯
每个对象都可以向队列中发送信息，同时检查有没有发给自己的信息
信息需要标注发送者和接收者，因此需要给所有对象设计一个一一对应的编码
使用一个32位二进制数作为编码（邮政编码postal_code）
*/

#pragma once

#include "Config.h"
#include <deque>
#include <fstream>
#include <algorithm>

class MessageQueueSubscriber;

class MessageQueue
{
	//变量
public:
	struct Mail
	{
		unsigned int from;
		unsigned int to;
		std::string message;
	};

private:
	static std::array<unsigned int, 12> postal_code_counter;	//用于自动分配种内编号，目前有7个class需要
	static std::vector<MessageQueueSubscriber*> subscribers;	//用于储存接收者
	static std::deque<Mail> mail_box;
	static std::vector<unsigned int> interactivable_subscriber;

public:
	/*
	编码规则如下：
	一个32位二进制数，被分为前6位与后10位
	前6位表示物品的种类（每个class记为一个种类）
	后26位表示该种类内的编号
	使用 (种间编号 << 26) + 种内编号 得到完整的编号
	*/

	//一些特殊编码
	static const unsigned int blank_code = 4294967295;	//unsigned int的最大值，内部保留编码，用于表示空邮件
	static const unsigned int game_code = 0;	//表示Game实例的编码，视为第0类，只有一个实例
	static const unsigned int scene_code = (2 << 26);	//表示Scene实例的编码，视为第2类，只有一个实例
	static const unsigned int player_code = (3 << 26);	//表示Player实例的编码，视为第3类，只有一个实例
	static const unsigned int flag_code = (4 << 26);	//表示Flag实例的编码，视为第4类，只有一个实例
	static const unsigned int page_code = (12 << 26);	//表示Page实例的编码
	static const unsigned int all_interactivable_code = (13 << 26);

	//一些种间编号，在计算完整编号时使用	interclassic_code
	static const unsigned char interface_header = 5;	//界面的编号
	static const unsigned char robot_header = 6;	//蘑菇怪的编号
	static const unsigned char spark_header = 7;	//金币的编号
	static const unsigned char dialogue_interface_header = 8;	//对话界面的编号
	static const unsigned char movable_wall_header = 9;			//可移动墙壁的编号
	static const unsigned char NPC_header = 10;	//NPC的编号
	static const unsigned char receptor_header = 11;	//所有感受器（拉杆，压力板等）的编号
	static const unsigned char laser_header = 14;
	static const unsigned char laser_gun_header = 15;
	static const unsigned char robot_factory_header = 16;
	static const unsigned char shadow_generator_header = 17;
	static const unsigned char shadow_header = 18;

private:
	static const Mail blank_mail;

	//函数
public:
	//将所有counter重置为0
	static void resetPostalCodeCounter();

	//获得种内编号
	static unsigned int getIntraclassicCode(const unsigned char interclassic_code);

	//获得完整编号
	static unsigned int getPostalCode(const unsigned char interclassic_code);

	//根据完整编号得到种间编号
	static unsigned char computeInterclassicCode(const unsigned int full_postal_code);

	//将信息发送到消息队列
	static void send(unsigned int from, unsigned int to, std::string message);
	static void send(Mail mail);
	static Mail checkMailBox(unsigned int to_code);

	////检查消息队列中是否有发给自己的信息
	//static Mail check(unsigned short id);

	static void registerSubscriber(MessageQueueSubscriber* subscriber);
	static void deregisterSubscriber(MessageQueueSubscriber* subscriber);
	static void registerInteractiveSubscriber(unsigned int subscriber);
	static void deregisterInteractiveSubscriber(unsigned int subscriber);

	static const short getMailBoxSize();

	static void savePostalCodeCounter(std::ofstream& file);
	static void loadPostalCodeCounter(std::ifstream& file);

	static void clear();
};


//所有可以发送和接受信息的类的父类，定义了邮政编码等等
class MessageQueueSubscriber
{
	//变量
protected:
	unsigned int postal_code;	//需要在子类的构造函数中初始化
	bool send_message;

	unsigned char remained_mail_counter;

	//函数
public:
	MessageQueueSubscriber();
	~MessageQueueSubscriber();

	const bool getSendMessage() const;
	const unsigned int getPostalCode() const;

	//简化的发送函数
	void send(unsigned int to, std::string message);
	virtual MessageQueue::Mail check();

	void increaseCounter();

protected:
	void checkMailBoxAndReact();
	void checkAllMail();
	void consumeMail();
	virtual void react();
};