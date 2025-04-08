/*

�Ի�dialogues->��ͼmaps->Ļacts->��������
��ҵĲ�ͬѡ�񽫵���ͬ��acts����һ��acts�ڰ��վ籾��������

��ÿ��actд��.json�ļ���������ScriptĿ¼����

����
- ����.json�ļ�
- ��õ�ͼ�����ƣ����ڼ��ص�ͼ
- ��öԻ��������Լ�ѡ����ڶԻ�ϵͳ
- ���ʵ��ĵط������Ի�
- ���ͨ�أ��������ӣ����л���һ�ŵ�ͼ������act��
- ������ҵ�ѡ���ۼ�ĳ����ֵ������senֵ�����Դ˾�������act������

˼·��
- ���ĳ�ֽ�������������.json�ļ�
- ���Dialogues����չʾ�Ի���ѡ��
- ������λ�ã����ڴ����Ի�
- ��Ҫ�������������㼶����Ϣ�Ľṹ

*/

#pragma once

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "Config.h"
#include "MessageQueueSubscriber.h"
#include <fstream>
#include <nlohmann/json.hpp>	//����.json�ļ�
#include <locale>
#include <codecvt>

namespace Script
{
	//�����������߲���������֮���ǲ��Ĺ�ϵ
	struct Expression
	{
		std::string variable;
		std::string i_operator;
		short value;
	};

	//һЩ���ڴ���һĻ����Ϣ�Ľṹ
	//����һ��ѡ��
	struct Choice
	{
		short choice_id;
		std::wstring text;
		short next_dialogue = -1;	//������һ��Ի�
		std::vector<Expression> effect;
	};

	//����һ��Ի����Լ�������ѡ��
	struct Dialogue
	{
		short dialogue_id;
		std::wstring speaker;
		std::wstring text;
		std::vector<Choice> choices;
		short next_dialogue = -1; //������һ��Ի���û��ѡ��ʱʹ��
		std::vector<Expression> effect;
	};

	struct DialogueTrigger
	{
		struct TriggerZone	//����ҽ�����ĳ����Χ���򴥷��Ի�
		{
			short left, right, top, bottom;
		}trigger_zone;
		short trigger_dialogue_id;	//�����ĶԻ���ID
		short trigger_id;	//��������ID
	};

	struct TimeLimit
	{
		int minute, second;
		std::wstring message;
	};

	//����һ����ͼ����Ϣ
	struct Map
	{
		short map_id;
		std::string name;	//���ڲ��ҵ�ͼ�ļ�
		std::wstring description;
		std::string background;	//���ڲ��ұ����ļ�
		std::string bgm;
		std::vector<Dialogue> dialogues;
		std::vector<DialogueTrigger> dialogue_triggers;
		std::vector<sf::FloatRect> quake_zone;
		short next_map;	//�����¸��ŵ�ͼ��-1��ʾ��Ļ����
		bool be_able_to_rewind_time;
		bool move_camera;
		bool global_mode;
		TimeLimit time_limit;
		std::string recorded_timeline_filename;	//����Ϊ�գ������HistoryĿ¼�ж�ȡ���ļ��������Ƶ�timeline.bin
	};

	//����һĻ����Ϣ
	struct Act
	{
		short act_id;
		std::wstring name;
		std::wstring description;
		std::vector<Map> maps;
	};

	//һЩ���ڴ���������Ľṹ
	//������һĻ��һ�ֿ�����
	struct NextAct
	{
		short next_act_id;
		std::vector<Expression> enter_condition;
	};

	//�����������һĻ����Ϣ
	struct ActInTree
	{
		short act_id;
		std::string act_file_name;
		std::vector<NextAct> next_acts;
		std::wstring end_name;	//�����ֵ�����
	};

	//���������ľ�����
	struct ScriptTree
	{
		std::vector<ActInTree> acts;
	};

	const std::string DEFAULT_BGM = "I_ClosedEyes";

    //����һĻ�Ľű�
	void parseActScript(const std::string& filename, Act& act);

	//����������
	void parseScriptTree(const std::string& filename, ScriptTree& script_tree);

	//ͨ��id����map�еĶԻ�
	Dialogue findDialogue(Map& i_map, short id);

	//ͨ��id����act
	ActInTree findAct(ScriptTree& i_script_tree, int id);

	//���ַ���ת��Ϊ���ַ���
	std::wstring stringToWstring(const std::string& str);

	//����Ƿ������������
	const bool checkEnterCondition(const NextAct& next_act);

	//����effect
	const bool parseEffect(const Expression& expression);
}