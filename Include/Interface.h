#pragma once

#include <functional>
#include <sstream>

#include "Config.h"
#include "Text.h"
#include "MessageQueueSubscriber.h"
#include "Script.h"

class Interface : public sf::Drawable, public sf::Transformable, public MessageQueueSubscriber
{
	//变量
protected:
	struct InterfaceElement	//用于储存界面中的一个元素
	{
		std::wstring message;	//内容
		sf::Vector2f center;	//相对于屏幕左上角的位置
		bool can_be_press = false;	//代表它是一个按钮还是只是一句话
		unsigned char character_size;	//字号
		int font_id;	//字体
		std::function<void()> function;	//点击按钮后执行
		bool align = true;	//是否与其他元素对齐
	};

	std::vector<InterfaceElement> element_list;	//用于储存所有的元素的信息

	std::vector<Text*> text_list;	//用于储存所有的元素

	sf::Vector2f position;	//屏幕左上角的位置

	unsigned char button_num;	//记录有多少个按钮
	unsigned char current_button;	//记录当前选择的按钮\

	unsigned char delay;
	unsigned char begin_delay;

	sf::RectangleShape* choose_box;	//选择框，用于框住当前选中的按钮

	const int DEFAULT_CHARACTER_SIZE = 22;
	
	//函数
public:
	Interface();
	~Interface();

	void updateViewCenter(sf::Vector2f view_center);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	virtual void update(sf::RenderWindow* window);

protected:
	void init();

	void setChooseBox();	//根据current_buttom设置choose_box的位置和大小

private:
	virtual void react() override;
};

class PauseInterface : public Interface
{
public:
	PauseInterface();

private:
	void reset();
	void backToGame();
	void saveAndExit();
};

class StartInterface : public Interface
{
public:
	StartInterface();
	~StartInterface();

private:
	void continueGame();
	void newGame();
	void confirmAndNewGame();
	void quit();
};

class ConfirmInterface : public Interface
{
public:
	std::string yes_message, no_message;
	ConfirmInterface(const std::string confirm_message, const std::string i_yes_message, const std::string i_no_message);
	~ConfirmInterface();

private:
	void yes();
	void no();
};

class DialogueInterface : public Interface
{
private:
	int next_dialogue_id;
	unsigned char dialogue_delay;	//控制对话中文字出现的延迟效果
	bool dialogue_text_finish;	//控制选项出现
	short char_num;	//控制文本出字
	Script::Dialogue dialogue;
	bool is_npc_dialogues;

	bool skip_dialogue;

	const short DEFAULT_DIALOGUE_CHARATER_SIZE = 20;
	const unsigned char DIALOGUE_SHOW_CHARACTER_DELAY = 2;

	void initChoices();

	virtual void react() override;

public:
	DialogueInterface(Script::Dialogue i_dialogue, bool i_is_npc_dialogues);

	virtual void update(sf::RenderWindow* window) override;

	const bool getIsNPCDialogues() const;
};