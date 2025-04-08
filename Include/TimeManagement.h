#pragma once

#include "Config.h"
#include "Player.h"
#include "Object.h"
#include "Enemy.h"
#include "ObjectManagement.h"
#include "Robot.h"
#include "File.h"
#include "Laser.h"

#include <deque>
#include <fstream>
#include <thread>
#include <mutex>
#include <future>
#include <unordered_set>

extern std::mutex fileMutex; // 互斥锁，防止多线程同时访问文件

namespace Snap
{
	//用于储存单个对象的瞬间状态
	struct ObjectSnap
	{
		sf::Vector2f coord;
		float horizontal_speed, vertical_speed;
		unsigned int postal_code;	//用于确定应该如何插入
		unsigned short eigen_code;	//特征编码
	};

	//用于储存所有对象的瞬间状态
	struct WorldSnap
	{
		std::deque<ObjectSnap> object;
		unsigned int s_points;
	};
}


class TimeManagement
{
	//变量
private:
	//用于储存一系列世界快照
	//使用双端队列，方便存入和删除
	std::deque<Snap::WorldSnap> world_history;

	//用于确认记录密度
	unsigned char counter;

	//文件读写相关
	std::future<void> save_task;
	std::future<void> load_task;
	std::atomic<bool> is_saving{ false };
	std::atomic<bool> is_loading{ false };

	struct FileSnapHeader
	{
		bool is_valid;          // 标记是否有效（true=有效）
		size_t object_count;    // 当前快照的对象数量
	};

	std::filesystem::path FILE_PATH;

	const size_t object_snap_size = sizeof(float) * 4 + sizeof(unsigned short) + sizeof(unsigned int);

public:
	static std::unordered_set<unsigned short> revive_black_list;

	//函数
	TimeManagement();
	TimeManagement(bool load);
	~TimeManagement();

	void clear();

	void record(Player* i_player, ObjectManagement* i_object_manager);
	void rewind(Player* i_player, ObjectManagement* i_object_manager);

private:
	void snap(Player* i_player, ObjectManagement* i_object_manager);
	void reviveSnap(Player* i_player, ObjectManagement* i_object_manager);
	void reviveSnap(Player* i_player, ObjectManagement* i_object_manager, float rate);

	//异步地存储和读取历史数据
	void asyncSaveToFile();
	void asyncLoadFromFile();
	void serializeSnap(const Snap::WorldSnap& snap, std::ofstream& file);
	Snap::WorldSnap deserializeSnap(std::fstream& file, bool& is_valid, size_t& world_snap_size);

public:
	void asyncSaveAllToFile();

	const unsigned int getSize() const;
};

