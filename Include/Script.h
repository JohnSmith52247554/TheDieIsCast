/*

对话dialogues->地图maps->幕acts->完整剧情
玩家的不同选择将导向不同的acts，而一个acts内按照剧本线性叙述

将每个act写成.json文件，保存在Script目录里面

需求：
- 解析.json文件
- 获得地图的名称，用于加载地图
- 获得对话的内容以及选项，用于对话系统
- 在适当的地方触发对话
- 玩家通关（拉下旗子）后切换下一张地图（或者act）
- 根据玩家的选择累计某种数值（例如sen值），以此决定后续act的走向

思路：
- 设计某种解析函数来解析.json文件
- 设计Dialogues类来展示对话和选项
- 检测玩家位置，用于触发对话
- 需要定义出保存各个层级的信息的结构

*/

#pragma once

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "Config.h"
#include "MessageQueueSubscriber.h"
#include <fstream>
#include <nlohmann/json.hpp>	//解析.json文件
#include <locale>
#include <codecvt>

namespace Script
{
	//储存条件或者操作，条件之间是并的关系
	struct Expression
	{
		std::string variable;
		std::string i_operator;
		short value;
	};

	//一些用于储存一幕内信息的结构
	//储存一个选项
	struct Choice
	{
		short choice_id;
		std::wstring text;
		short next_dialogue = -1;	//决定下一句对话
		std::vector<Expression> effect;
	};

	//储存一句对话，以及附属的选项
	struct Dialogue
	{
		short dialogue_id;
		std::wstring speaker;
		std::wstring text;
		std::vector<Choice> choices;
		short next_dialogue = -1; //决定下一句对话，没有选项时使用
		std::vector<Expression> effect;
	};

	struct DialogueTrigger
	{
		struct TriggerZone	//若玩家进入了某个范围，则触发对话
		{
			short left, right, top, bottom;
		}trigger_zone;
		short trigger_dialogue_id;	//触发的对话的ID
		short trigger_id;	//触发器的ID
	};

	struct TimeLimit
	{
		int minute, second;
		std::wstring message;
	};

	//储存一个地图的信息
	struct Map
	{
		short map_id;
		std::string name;	//用于查找地图文件
		std::wstring description;
		std::string background;	//用于查找背景文件
		std::string bgm;
		std::vector<Dialogue> dialogues;
		std::vector<DialogueTrigger> dialogue_triggers;
		std::vector<sf::FloatRect> quake_zone;
		short next_map;	//决定下个张地图，-1表示本幕结束
		bool be_able_to_rewind_time;
		bool move_camera;
		bool global_mode;
		TimeLimit time_limit;
		std::string recorded_timeline_filename;	//若不为空，则会在History目录中读取此文件，并复制到timeline.bin
	};

	//储存一幕的信息
	struct Act
	{
		short act_id;
		std::wstring name;
		std::wstring description;
		std::vector<Map> maps;
	};

	//一些用于储存剧情树的结构
	//储存下一幕的一种可能性
	struct NextAct
	{
		short next_act_id;
		std::vector<Expression> enter_condition;
	};

	//储存剧情树中一幕的信息
	struct ActInTree
	{
		short act_id;
		std::string act_file_name;
		std::vector<NextAct> next_acts;
		std::wstring end_name;	//储存结局的名字
	};

	//储存完整的剧情树
	struct ScriptTree
	{
		std::vector<ActInTree> acts;
	};

	const std::string DEFAULT_BGM = "I_ClosedEyes";

    //解析一幕的脚本
	void parseActScript(const std::string& filename, Act& act);

	//解析剧情树
	void parseScriptTree(const std::string& filename, ScriptTree& script_tree);

	//通过id查找map中的对话
	Dialogue findDialogue(Map& i_map, short id);

	//通过id查找act
	ActInTree findAct(ScriptTree& i_script_tree, int id);

	//将字符串转化为宽字符串
	std::wstring stringToWstring(const std::string& str);

	//检查是否满足进入条件
	const bool checkEnterCondition(const NextAct& next_act);

	//解析effect
	const bool parseEffect(const Expression& expression);
}