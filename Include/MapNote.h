/*

地图注释
解析.json文件，并依据其中的信息生成具有复杂行为的实体
以此弥补.tmx无法存储足够的数据的问题

*/

#pragma once

#include "Config.h"
#include "Script.h"
#include <nlohmann/json.hpp>

namespace MapNote
{
	enum ReceptorType
	{
		drawbar = 0,	//拉杆
		button,			//按钮
		pressure_plate,	//压力板
		key,			//钥匙
		helmet
	};

	//一些储存数据的数据结构
	struct MovableWall
	{
		short x, y;	//此处的坐标均以整格（32*32）为单位
		Direction structure_direction;	//控制墙壁是水平的还是竖直的
		short length;
		Direction move_direction;
		short move_distance;
		float speed;
		bool set_can_be_affected_by_time_control;
	};

	struct ReceptorAndEffectors	//由感受器（拉杆等）和效应器（可为多个）组成
	{
		struct Receptor
		{
			ReceptorType type;
			std::string activation_message;	//被激活时发送信息
			std::string release_message;	//回到普通状态时发送信息
			short x, y;
			bool set_can_be_affected_by_time_control;
		} receptor;
		std::vector<unsigned int>	effector_postal_code;	//效应器的邮编，需要小心的倒推
		//如果需要让不同的效应器做出不同的反应，需要将两个message改为字符串向量
	};

	struct NPC
	{
		short npc_id;
		short npc_skin_id;
		std::wstring name;
		short x, y;
		short dialogue_id_while_rewinding_time;
		std::vector<Script::Dialogue> dialogues;
	};

	struct Note
	{
		std::vector<MovableWall> movable_walls;
		std::vector<ReceptorAndEffectors> receptors_and_effectors;
		std::vector<NPC> npcs;
	};

	void parseMapNote(const std::string& filename, Note& map_note);

	int stringToEnum(const std::string& str);
}