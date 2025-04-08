/*

游戏的本体
将会创建scene，作为游戏的场景
原来的Game的很多功能将转移到Scene中

*/

#pragma once

#include "Config.h"
#include "Interface.h"
#include "MessageQueueSubscriber.h"
#include "Scene.h"
#include "Script.h"
#include "Page.h"
#include "File.h"

#include <list>

class Game : public MessageQueueSubscriber
{
//变量
protected:
	sf::RenderWindow* window;
	sf::Event ev;

	Page* page;

	Scene* scene;

	Script::Act act;	//储存剧情信息（幕内）
	Script::ScriptTree script_tree;	//储存剧情信息（幕间）
	Script::ActInTree act_in_tree;
	int current_act_id;
	int current_scene_id;	//记录当前是第几关

	sf::Image icon;

#if SHOW_CONSOLE
	struct ActScene
	{
		int act_id;
		int scene_id;
	};
	std::list<ActScene> act_scene_recorder;
#endif

	enum windowState
	{
		fullscreen,
		notfullscreen
	};
	windowState window_state;
	unsigned char change_window_state_delay;

//函数
private:
	//初始化
	void initVar();
	void initWin();
	void initObject();

	//事件轮询
	void pollEvent();

	//处理邮件
	virtual void react() override;

public:
	//构造与析构函数
	Game();
	~Game();
	
	//传出函数
	bool isWindowOpen();

	//更新函数
	void update();

	//渲染函数
	void render();

	void waitForClose();

private:
	void save();
	void load();
};