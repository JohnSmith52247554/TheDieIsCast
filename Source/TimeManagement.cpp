#include "TimeManagement.h"

using namespace Snap;

std::mutex fileMutex;
std::unordered_set<unsigned short> TimeManagement::revive_black_list;

TimeManagement::TimeManagement()
{
	FILE_PATH = EXE_DIR / "History" / "timeline.bin";

	counter = RECORD_DENSITY;

	//清空文件
	std::ofstream file(FILE_PATH, std::ios::binary | std::ios::trunc);
	if (file.is_open()) 
	{
		file.close();
	}
}

TimeManagement::TimeManagement(bool load)
{
	FILE_PATH = EXE_DIR / "History" / "timeline.bin";
	counter = RECORD_DENSITY;
	if (load == false)
	{
		//清空文件
		std::ofstream file(FILE_PATH, std::ios::binary | std::ios::trunc);
		if (file.is_open())
		{
			file.close();
		}
	}
}

TimeManagement::~TimeManagement()
{
	revive_black_list.clear();
}

void TimeManagement::clear()
{
	//清除所有快照
	world_history.clear();
	//清空文件
	std::ofstream file(FILE_PATH, std::ios::binary | std::ios::trunc);
	if (file.is_open())
	{
		file.close();
	}
}

void TimeManagement::record(Player* i_player, ObjectManagement* i_object_manager)
{
	//以一定频率记录快照
	if (counter >= RECORD_DENSITY)
	{
		snap(i_player, i_object_manager);
		counter = 1;
	}
	else
	{
		counter++;
	}

	/*std::cout << world_history.size() << '\n';*/
}

void TimeManagement::rewind(Player* i_player, ObjectManagement* i_object_manager)
{
	//以一定频率复现快照
	if (world_history.size() <= MAX_RECORD_TIME * FPS / RECORD_DENSITY / SAVE_AND_LOAD_RATIO)
	{
		//尝试从文件中读取快照
		asyncLoadFromFile();
	}

	if (counter == 1)
	{
		counter = RECORD_DENSITY;
		if (world_history.size() > 0)
		{
			world_history.pop_back();
		}
	}
	else
	{	
		counter--;
	}

	//使用插值法让效果更加平滑
	reviveSnap(i_player, i_object_manager, static_cast<float>(counter - 1) / static_cast<float>(RECORD_DENSITY));

	/*std::cout << world_history.size() << '\n';*/
}

void TimeManagement::snap(Player* i_player, ObjectManagement* i_object_manager)
{
	std::vector<Enemy*> i_enemy_list = i_object_manager->getEnemyList();
	std::vector<Object*> i_object_list = i_object_manager->getObjectList();

	WorldSnap world_snap;

	world_snap.s_points = points;

	unsigned int num = 0;
	if (i_player->canBeAffectedByTimeControl())
	{
		ObjectSnap object_snap;
		object_snap.coord = i_player->getCoord();
		object_snap.horizontal_speed = i_player->getHorizontalSpeed();
		object_snap.vertical_speed = i_player->getVerticalSpeed();
		object_snap.postal_code = i_player->getPostalCode();
		object_snap.eigen_code = i_player->encode();
		world_snap.object.push_back(std::move(object_snap));
		//将玩家数据储存在第一位
		//接下来储存敌人信息和其他实体的信息
	}

	for (auto& enemy : i_enemy_list)
	{
		if (enemy->canBeAffectedByTimeControl())
		{
			ObjectSnap object_snap;
			object_snap.coord = enemy->getCoord();
			object_snap.horizontal_speed = enemy->getHorizontalSpeed();
			object_snap.vertical_speed = enemy->getVerticalSpeed();
			object_snap.postal_code = enemy->getPostalCode();
			object_snap.eigen_code = enemy->encode();
			world_snap.object.push_back(std::move(object_snap));
		}
	}

	for (auto& object : i_object_list)
	{
		if (object->canBeAffectedByTimeControl())
		{
			ObjectSnap object_snap;
			object_snap.coord = object->getCoord();
			object_snap.horizontal_speed = object->getHorizontalSpeed();
			object_snap.vertical_speed = object->getVerticalSpeed();
			object_snap.postal_code = object->getPostalCode();
			object_snap.eigen_code = object->encode();
			//world_snap.object.push_back(object_snap);
			world_snap.object.push_back(std::move(object_snap));
		}
	}

	world_history.push_back(std::move(world_snap));

	if (world_history.size() > MAX_RECORD_TIME * FPS / RECORD_DENSITY)
	{
		//不再删除数据，而是将数据记录在文档中
		asyncSaveToFile();
	}
}

void TimeManagement::reviveSnap(Player* i_player, ObjectManagement* i_object_manager)
{
	if (world_history.size() < 1)
		return;

	//根据储存的快照复现过去实体的位置和速度
	WorldSnap& world_snap = world_history.at(world_history.size() - 1);
	points = world_snap.s_points;

	unsigned int pos = 0;

	//复现玩家
	if (i_player->canBeAffectedByTimeControl())
	{
		ObjectSnap& object_snap = world_snap.object.at(0);
		if (object_snap.postal_code == MessageQueue::player_code)
		{
			i_player->setState(object_snap.coord, object_snap.horizontal_speed, object_snap.vertical_speed);
			i_player->decode(object_snap.eigen_code);
			pos++;
		}
	}

	//复现敌人和其他实体
	const std::vector<Enemy*> i_enemy_list = i_object_manager->getEnemyList();
	const std::vector<Object*> i_object_list = i_object_manager->getObjectList();
	unsigned short enemy_counter = 0;

	//用于在敌人未生成前抹除敌人
	std::vector<std::pair<Enemy*, bool>> enemy_list_and_exist;
	enemy_list_and_exist.resize(i_enemy_list.size());
	for (int i = 0; i < i_enemy_list.size(); i++)
	{
		enemy_list_and_exist.at(i).first = i_enemy_list.at(i);
		if (i_enemy_list.at(i)->canBeAffectedByTimeControl())
		{
			enemy_list_and_exist.at(i).second = false;
		}
		else
		{
			enemy_list_and_exist.at(i).second = true;
		}
	}

	//根据邮政编码在将信息传递给正确的对象
	for (int i = pos; i < world_snap.object.size(); i++)
	{
		ObjectSnap& object_snap = world_snap.object.at(i);

		if (enemy_counter < enemy_list_and_exist.size())
		{
			for (auto& enemy : enemy_list_and_exist)
			{
				//根据邮政编码查找
				if (enemy.first->getPostalCode() == object_snap.postal_code)
				{
					if (enemy.first->canBeAffectedByTimeControl() == true)
					{
						enemy.first->setState(object_snap.coord, object_snap.horizontal_speed, object_snap.vertical_speed);
						enemy.first->decode(object_snap.eigen_code);
					}
					enemy_counter++;
					enemy.second = true;	//标记为已出现
					goto LOOP_END;
				}
			}
		}

		for (auto& object : i_object_list)
		{
			if (object->canBeAffectedByTimeControl() == false)
				continue;

			if (object->getPostalCode() == object_snap.postal_code)
			{
				object->setState(object_snap.coord, object_snap.horizontal_speed, object_snap.vertical_speed);
				object->decode(object_snap.eigen_code);
				goto LOOP_END;
			}
		}

		//未找到邮政编码匹配的，生成并赋值
		//生成蘑菇怪
		if (revive_black_list.find(object_snap.postal_code) != revive_black_list.end())
			continue;
		if (MessageQueue::computeInterclassicCode(object_snap.postal_code) == MessageQueue::robot_header)
		{
			Robot* g = new Robot(object_snap.coord.x, object_snap.coord.y, i_object_manager, object_snap.postal_code);
			g->setState(object_snap.coord, object_snap.horizontal_speed, object_snap.vertical_speed);
			g->decode(object_snap.eigen_code);
		}
		else if (MessageQueue::computeInterclassicCode(object_snap.postal_code) == MessageQueue::laser_header)
		{
			Laser* l = new Laser(object_snap.coord.x, object_snap.coord.y, i_object_manager, right, object_snap.postal_code);
			l->setState(object_snap.coord, object_snap.horizontal_speed, object_snap.vertical_speed);
			l->decode(object_snap.eigen_code);
		}

	LOOP_END:
		continue;
	}

	//抹除不存在的实体
	for (auto enemy : enemy_list_and_exist)
	{
		if (enemy.second == false)
		{
			i_object_manager->eraseEnemy(enemy.first);
		}
	}
}

void TimeManagement::reviveSnap(Player* i_player, ObjectManagement* i_object_manager, float rate)
{
	
	if (world_history.size() < 2)
	{
		reviveSnap(i_player, i_object_manager);
		return;
	}

	//根据储存的快照复现过去实体的位置和速度
	WorldSnap& world_snap_1 = world_history.at(world_history.size() - 1);
	WorldSnap& world_snap_2 = world_history.at(world_history.size() - 2);
	points = world_snap_1.s_points;

	sf::Vector2f coord;
	//复现玩家

	unsigned int pos = 0;
	if(i_player->canBeAffectedByTimeControl())
	{
		ObjectSnap& object_snap_1 = world_snap_1.object.at(0);
		if (object_snap_1.postal_code == MessageQueue::player_code)
		{
			ObjectSnap& object_snap_2 = world_snap_2.object.at(0);
			coord =
			{
				object_snap_1.coord.x * rate + object_snap_2.coord.x * (1 - rate),
				object_snap_1.coord.y * rate + object_snap_2.coord.y * (1 - rate)
			};

			i_player->setState(coord, object_snap_1.horizontal_speed, object_snap_1.vertical_speed);
			i_player->decode(object_snap_1.eigen_code);
			pos++;
		}
	}

	//复现敌人和其他实体
	std::vector<Enemy*> i_enemy_list = i_object_manager->getEnemyList();
	std::vector<Object*> i_object_list = i_object_manager->getObjectList();
	unsigned short enemy_counter = 0;

	//用于在敌人未生成前抹除敌人
	std::vector<std::pair<Enemy*, bool>> enemy_list_and_exist;
	enemy_list_and_exist.resize(i_enemy_list.size());
	for (int i = 0; i < i_enemy_list.size(); i++)
	{
		enemy_list_and_exist.at(i).first = i_enemy_list.at(i);
		if (i_enemy_list.at(i)->canBeAffectedByTimeControl())
		{
			enemy_list_and_exist.at(i).second = false;
		}
		else
		{
			enemy_list_and_exist.at(i).second = true;
		}
	}

	for (int i = pos; i < world_snap_1.object.size(); i++)
	{
		ObjectSnap& object_snap_1 = world_snap_1.object.at(i);
		for (int j = pos; j < world_snap_2.object.size(); j++)
		{
			//查找相同的postal_code
			ObjectSnap& object_snap_2 = world_snap_2.object.at(j);
			if (object_snap_1.postal_code != object_snap_2.postal_code)
				continue;

			//复现
			coord =
			{
				object_snap_1.coord.x * rate + object_snap_2.coord.x * (1 - rate),
				object_snap_1.coord.y * rate + object_snap_2.coord.y * (1 - rate)
			};

			if (enemy_counter < enemy_list_and_exist.size())
			{
				for (auto& enemy : enemy_list_and_exist)
				{
					//根据邮政编码查找
					if (enemy.first->getPostalCode() == object_snap_1.postal_code)
					{
						if (enemy.first->canBeAffectedByTimeControl() == true)
						{
							enemy.first->setState(coord, object_snap_1.horizontal_speed, object_snap_1.vertical_speed);
							enemy.first->decode(object_snap_1.eigen_code);
						}
						enemy_counter++;
						enemy.second = true;	//标记为已出现
						goto LOOP_END;
					}
				}
			}

			for (auto& object : i_object_list)
			{
				if (object->canBeAffectedByTimeControl() == false)
					continue;

				if (object->getPostalCode() == object_snap_1.postal_code)
				{
					object->setState(coord, object_snap_1.horizontal_speed, object_snap_1.vertical_speed);
					object->decode(object_snap_1.eigen_code);
					goto LOOP_END;
				}
			}

			//未找到邮政编码匹配的，生成并赋值
			//生成蘑菇怪
			if (revive_black_list.find(object_snap_1.postal_code) != revive_black_list.end())
				continue;
			if (MessageQueue::computeInterclassicCode(object_snap_1.postal_code) == MessageQueue::robot_header)
			{
				Robot* g = new Robot(coord.x, coord.y, i_object_manager, object_snap_1.postal_code);
				g->setState(coord, object_snap_1.horizontal_speed, object_snap_1.vertical_speed);
				g->decode(object_snap_1.eigen_code);

			}
			else if (MessageQueue::computeInterclassicCode(object_snap_1.postal_code) == MessageQueue::laser_header)
			{
				Laser* l = new Laser(coord.x, coord.y, i_object_manager, right, object_snap_1.postal_code);
				l->setState(coord, object_snap_1.horizontal_speed, object_snap_1.vertical_speed);
				l->decode(object_snap_1.eigen_code);
			}

		LOOP_END:
			break;
		}
	}

	//抹除不存在的实体
	for (auto enemy : enemy_list_and_exist)
	{
		if (enemy.second == false)
		{
			i_object_manager->eraseEnemy(enemy.first);
			//i_enemy_list = i_object_manager->getEnemyList();
			//std::cout << "1\n";
		}
	}
}

// 异步存储历史数据到文件
void TimeManagement::asyncSaveToFile()
{
	if (world_history.empty() || is_saving) 
		return;
	is_saving = true;

	save_task = std::async(std::launch::async, [this]()
		{
			std::lock_guard<std::mutex> lock(fileMutex);

			size_t save_count = world_history.size() / SAVE_AND_LOAD_RATIO;
			if (save_count == 0) 
				return;

			std::ofstream file(FILE_PATH, std::ios::binary | std::ios::app);
			if (!file.is_open()) 
				return;

			// 写入数据，并标记为有效
			for (size_t i = 0; i < save_count; ++i) 
			{
				serializeSnap(world_history.front(), file);
				world_history.pop_front();
			}
			file.close();
			is_saving = false;
		});
}

void TimeManagement::asyncSaveAllToFile()
{
	if (world_history.empty() || is_saving)
		return;
	is_saving = true;

	save_task = std::async(std::launch::async, [this]()
		{
			std::lock_guard<std::mutex> lock(fileMutex);

			size_t save_count = world_history.size();
			if (save_count == 0)
				return;

			std::ofstream file(FILE_PATH, std::ios::binary | std::ios::app);
			if (!file.is_open())
			{
				is_saving = false;
				return;
			}

			// 写入数据，并标记为有效
			for (size_t i = 0; i < save_count; ++i)
			{
				serializeSnap(world_history.front(), file);
				world_history.pop_front();
			}
			file.close();
			is_saving = false;
		});
}

// 异步从文件加载历史数据（从后往前读）
void TimeManagement::asyncLoadFromFile()
{
	if (is_loading)
		return;
	is_loading = true;

	load_task = std::async(std::launch::async, [this]() 
		{
		try {
			std::lock_guard<std::mutex> lock(fileMutex);
			std::fstream file(FILE_PATH, std::ios::binary | std::ios::in | std::ios::out);
			if (!file.is_open()) 
			{
				is_loading = false;
				return;
			}

			file.seekg(0, std::ios::end);
			size_t total_size = file.tellg();
			size_t target_load_count = MAX_RECORD_TIME * FPS / RECORD_DENSITY / SAVE_AND_LOAD_RATIO;

			std::vector<WorldSnap> temp_storage;
			bool is_valid;
			size_t world_snap_size;

			// 从后向前扫描，读取有效快照
			while (temp_storage.size() < target_load_count && total_size > 0) 
			{
				if (total_size < sizeof(FileSnapHeader))
					break;

				// 先读取头部（在文件末尾）
				file.seekg(total_size - sizeof(FileSnapHeader));
				WorldSnap snap = deserializeSnap(file, is_valid, world_snap_size);

				if (is_valid) 
				{
					temp_storage.push_back(snap);
				}

				total_size -= world_snap_size + sizeof(FileSnapHeader); // 向前移动
			}

			if (temp_storage.size() > 0)
			{
				// 顺序插入到内存队列
				for (auto it = temp_storage.begin(); it != temp_storage.end(); ++it)
				{
					world_history.push_front(*it);
				}
			}

			file.close();
		}
		catch (...) 
		{
			
		}
		is_loading = false;
		});
}

// 序列化单个快照
void TimeManagement::serializeSnap(const WorldSnap& snap, std::ofstream& file) 
{
	file.write(reinterpret_cast<const char*>(&snap.s_points), sizeof(snap.s_points));

	// 先写入每个对象的数据
	for (const auto& obj : snap.object) 
	{
		file.write(reinterpret_cast<const char*>(&obj.coord.x), sizeof(float));
		file.write(reinterpret_cast<const char*>(&obj.coord.y), sizeof(float));
		file.write(reinterpret_cast<const char*>(&obj.horizontal_speed), sizeof(float));
		file.write(reinterpret_cast<const char*>(&obj.vertical_speed), sizeof(float));
		file.write(reinterpret_cast<const char*>(&obj.postal_code), sizeof(unsigned int));
		file.write(reinterpret_cast<const char*>(&obj.eigen_code), sizeof(unsigned short));
	}

	FileSnapHeader header;
	header.is_valid = true;  // 标记为有效
	header.object_count = snap.object.size();

	// 后写入头部
	file.write(reinterpret_cast<const char*>(&header), sizeof(FileSnapHeader));
}

// 反序列化时检查有效性
WorldSnap TimeManagement::deserializeSnap(std::fstream& file, bool& is_valid, size_t& world_snap_size)
{
	//读取头部
	FileSnapHeader header;
	file.read(reinterpret_cast<char*>(&header), sizeof(FileSnapHeader));
	is_valid = header.is_valid;
	world_snap_size = header.object_count * object_snap_size + sizeof(unsigned int);

	WorldSnap snap;
	if (is_valid) 
	{
		//标记为无效
		header.is_valid = false;
		file.seekg(-static_cast<int>(sizeof(FileSnapHeader)), std::ios::cur);
		file.write(reinterpret_cast<const char*>(&header), sizeof(FileSnapHeader));

		//向前移动以读取内容
		file.seekg(-static_cast<int>(world_snap_size) - static_cast<int>(sizeof(FileSnapHeader)), std::ios::cur);

		file.read(reinterpret_cast<char*>(&snap.s_points), sizeof(snap.s_points));

		for (size_t i = 0; i < header.object_count; ++i)
		{
			ObjectSnap obj;
			file.read(reinterpret_cast<char*>(&obj.coord.x), sizeof(float));
			file.read(reinterpret_cast<char*>(&obj.coord.y), sizeof(float));
			file.read(reinterpret_cast<char*>(&obj.horizontal_speed), sizeof(float));
			file.read(reinterpret_cast<char*>(&obj.vertical_speed), sizeof(float));
			file.read(reinterpret_cast<char*>(&obj.postal_code), sizeof(unsigned int));
			file.read(reinterpret_cast<char*>(&obj.eigen_code), sizeof(unsigned short));
			snap.object.push_back(obj);
		}
	}

	//向前移动至下一个header
	file.seekg(-static_cast<int>(world_snap_size), std::ios::cur);

	return snap;
}

const unsigned int TimeManagement::getSize() const
{
	return world_history.size();
}