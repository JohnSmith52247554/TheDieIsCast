#include "ShadowGenerator.h"

ShadowGenerator::ShadowGenerator(int x, int y, ObjectManagement* object_manager, bool load)
	: ShadowGenerator(x, y, object_manager, load, MessageQueue::blank_code)
{
	postal_code = MessageQueue::getPostalCode(MessageQueue::shadow_generator_header);
	std::stringstream ss;
	ss << std::hex << postal_code;
	FILE_PATH = EXE_DIR / "History" / (ss.str() + ".bin");
	if (load == false)
	{
		std::ofstream file(FILE_PATH, std::ios::binary);
		file.close();
	}
}

ShadowGenerator::ShadowGenerator(int x, int y, ObjectManagement* object_manager, bool load, unsigned int setted_postal_code)
	: Object(x, y)
{
	object_manager->pushBackObject(this);
	save_manager = object_manager;

	shadow_exist = false;
	hitted = false;
	prev_hitted = false;
	listening = false;
	rewind_shadow = false;
	saved = false;
	list_ptr = 0U;

	gen_shadow_count = 0U;

	shadow_postal_code = MessageQueue::blank_code;

	postal_code = setted_postal_code;
	if (postal_code != MessageQueue::blank_code)
	{
		std::stringstream ss;
		ss << std::hex << postal_code;
		FILE_PATH = EXE_DIR / "History" / (ss.str() + ".bin");
		if (load == false)
		{
			std::ofstream file(FILE_PATH, std::ios::binary);
			file.close();
		}
	}

	initTexture(16);
	this->sprite.setTextureRect({ 0, 0, 64, 96 });
	this->sprite.setOrigin(32, 48);

	float i_y = coord.y - 48;
	while (MapCollision::getCell(coord.x, i_y, false) != Wall)
	{
		i_y -= CELL_SIZE;
	}
	i_y += CELL_SIZE;

	hit_box.width = 64;
	hit_box.height = (coord.y + 48) - i_y;
	hit_box.left = coord.x - 32;
	hit_box.top = i_y;

	auto shader_path = EXE_DIR / "Resources" / "Shader" / "noise.frag";
	if (!noise_shader.loadFromFile(shader_path.string(), sf::Shader::Fragment))
		std::cerr << "Failed to load shader" << std::endl;
}

ShadowGenerator::~ShadowGenerator()
{
	if (saved == false && std::filesystem::exists(FILE_PATH))
	{
		std::filesystem::remove(FILE_PATH);
	}
}

void ShadowGenerator::registerListener(player_listener listener)
{
	listen_player = listener;
}

void ShadowGenerator::registerListener(interactivable_listener listener)
{
	listen_interactivable.push_back(listener);
}

void ShadowGenerator::listen()
{
	float pvc, phc;
	unsigned short pec;
	unsigned int ic;

	listen_player(pvc, phc, pec);
 	ic = MessageQueue::blank_code;
	for (auto& listener : listen_interactivable)
	{
		listener(ic);
		if (ic != MessageQueue::blank_code)
			break;
	}

	ShadowSnap snap =
	{
		pvc, phc,
		pec,
		ic,
	};

	record_list.push_back(snap);

	//list_size++;

	//std::cout << "listen " << list_size << '\n';
}

void ShadowGenerator::update()
{
	prev_hitted = hitted;
	hitted = false;
	checkMailBoxAndReact();

	if (hitted == false && prev_hitted == true)
	{
		if (listening == true)
		{
			//生成影子
			auto shadow = new Shadow(coord.x, coord.y, save_manager, [this](float& pvc, float& phc, unsigned short& pec, unsigned int& ic)
				{this->getRecord(pvc, phc, pec, ic); });
			shadow_postal_code = shadow->getPostalCode();
			rewind_shadow = false;
			shadow_exist = true;

			listening = false;
		}
		else if (shadow_exist == false)
			listening = true;
	}

	if (shadow_exist == true)
	{
		if (record_list.size() - list_ptr <= 1)
		{
			shadow_exist = false;
			send(shadow_postal_code, "vanish");
			//begin_and_end_list.push_back({ shadow_begin, shadow_end });
			if (rewind_shadow == false)
			{
				all_shadow_record.push_back(std::move(record_list));
				if (all_shadow_record.size() >= 2)
					asyncSaveRecord();
			}
			else
			{
				record_list.clear();
			}
			gen_shadow_count++;
			list_ptr = 0U;
			//std::cout << "gen_shadow_count: " << gen_shadow_count << '\n';
		}
		else
			list_ptr++;
	}

	if (listening)
		listen();
}

const sf::FloatRect* ShadowGenerator::getHitBox()
{
	return &hit_box;
}

const unsigned short ShadowGenerator::encode() const
{
	return (gen_shadow_count << 1) + listening;
}

void ShadowGenerator::decode(const unsigned short i_eigen_code)
{
	if (!listening && i_eigen_code & 1)
		record_list.clear();
	listening = i_eigen_code & 1;

	if (shadow_exist)
	{
		if (list_ptr > 0)
			list_ptr--;
		else
		{
			shadow_exist = false;
			send(shadow_postal_code, "vanish");

			record_list.clear();
			list_ptr = 0U;
		}
	}
	else
	{
		if (gen_shadow_count > ((i_eigen_code >> 1) & 7))
		{
			if (all_shadow_record.size() > 0)
			{
				record_list = std::move(all_shadow_record.back());
				all_shadow_record.pop_back();
			}
			list_ptr = record_list.size() - 2;
			gen_shadow_count--;
			if (gen_shadow_count > 0)
				asyncLoadRecord();
			
			auto shadow = new Shadow(coord.x, coord.y, save_manager, [this](float& pvc, float& phc, unsigned short& pec, unsigned int& ic)
				{this->getRecord(pvc, phc, pec, ic); });
			shadow_postal_code = shadow->getPostalCode();
			rewind_shadow = true;
			shadow_exist = true;
			listening = false;
		}
		else
		{
			if (record_list.size() > 0)
				record_list.pop_back();
		}
	}
}

void ShadowGenerator::react()
{
	MessageQueue::Mail mail = check();

	if (mail.from == MessageQueue::blank_code)
		return;

	if (mail.from == MessageQueue::player_code)
	{
		if (mail.message == "hitted by player")
		{
			hitted = true;
		}
	}
}

void ShadowGenerator::getRecord(float& pvc, float& phc, unsigned short& pec, unsigned int& ic)
{
	if (list_ptr < 0 && list_ptr >= record_list.size())
		return;
	const auto& record = record_list.at(list_ptr);

	pvc = record.player_vertical_coord;
	phc = record.player_horizontal_coord;
	pec = record.player_eigen_code;
	ic = record.interact_postal_code;
}

void ShadowGenerator::asyncSaveRecord()
{
	if (all_shadow_record.size() == 0 || is_saving)
		return;
	is_saving = true;

	save_task = std::async(std::launch::async, [this]()
		{
			std::lock_guard<std::mutex> lock(file_mutex);

			std::ofstream file(FILE_PATH, std::ios::binary | std::ios::app);
			if (!file.is_open())
			{
				is_saving = false;
				return;
			}

			std::deque<ShadowSnap> record = std::move(all_shadow_record.front());
			all_shadow_record.pop_front();

			for (auto& snap : record)
			{
				file.write(reinterpret_cast<char*>(&snap.player_vertical_coord), sizeof(snap.player_vertical_coord));
				file.write(reinterpret_cast<char*>(&snap.player_horizontal_coord), sizeof(snap.player_horizontal_coord));
				file.write(reinterpret_cast<char*>(&snap.player_eigen_code), sizeof(snap.player_eigen_code));
				file.write(reinterpret_cast<char*>(&snap.interact_postal_code), sizeof(snap.interact_postal_code));
			}

			//写入头部
			//队列长度
			size_t size = record.size();
			file.write(reinterpret_cast<char*>(&size), sizeof(size));
			//设置为有效
			bool value = true;
			file.write(reinterpret_cast<char*>(&value), sizeof(value));

			file.close();

			is_saving = false;
		}
	);
}

void ShadowGenerator::asyncLoadRecord()
{
	//std::cout << "load\n";
	if (is_loading)
		return;
	is_loading = true;

	load_task = std::async(std::launch::async, [this]() {
		std::lock_guard<std::mutex> lock(file_mutex);

		std::fstream file(FILE_PATH, std::ios::binary | std::ios::in | std::ios::out);
		if (!file.is_open())
		{
			is_loading = false;
			return;
		}

		file.seekg(0, std::ios::end);
		size_t total_size = file.tellg();

		size_t size = 0;
		bool valid;

		while (total_size > 0)
		{
			file.seekg(total_size - (sizeof(size) + sizeof(valid)));
			file.read(reinterpret_cast<char*>(&size), sizeof(size));
			file.read(reinterpret_cast<char*>(&valid), sizeof(valid));

			total_size -= sizeof(size) + sizeof(bool) + size * SNAP_FILE_SIZE;
			if (valid)
				break;
		}

		file.seekg(total_size);

		std::deque<ShadowSnap> load_record;
		load_record.resize(size);
		for (auto& snap : load_record)
		{
			file.read(reinterpret_cast<char*>(&snap.player_vertical_coord), sizeof(snap.player_vertical_coord));
			file.read(reinterpret_cast<char*>(&snap.player_horizontal_coord), sizeof(snap.player_horizontal_coord));
			file.read(reinterpret_cast<char*>(&snap.player_eigen_code), sizeof(snap.player_eigen_code));
			file.read(reinterpret_cast<char*>(&snap.interact_postal_code), sizeof(snap.interact_postal_code));
		}
		valid = false;
		file.seekg(sizeof(size), std::ios::cur);
		file.write(reinterpret_cast<char*>(&valid), sizeof(valid));

		all_shadow_record.push_front(std::move(load_record));

		file.close();

		is_loading = false;
		}
	);
}

const unsigned short ShadowGenerator::saveCode()
{
	saved = true;

	if (record_list.size() > 0)
		all_shadow_record.push_back(record_list);

	while (all_shadow_record.size() > 0)
	{
		asyncSaveRecord();
		save_task.wait();
	}
	std::ofstream file(FILE_PATH, std::ios::binary | std::ios::out | std::ios::app);
	file.write(reinterpret_cast<char*>(&list_ptr), sizeof(list_ptr));	//伪装成正常的Snap
	for (int i = 0; i < SNAP_FILE_SIZE - sizeof(list_ptr); i++)
	{
		char temp = 0;
		file.write(reinterpret_cast<char*>(&temp), sizeof(temp));
	}

	size_t size = 1;
	bool valid = false;
	file.write(reinterpret_cast<char*>(&size), sizeof(size));
	file.write(reinterpret_cast<char*>(&valid), sizeof(valid));
	file.close();

	return gen_shadow_count + (listening << 8) + (shadow_exist << 9) + (rewind_shadow << 10);
}

void ShadowGenerator::praseSaveCode(const unsigned short save_code)
{
	gen_shadow_count = save_code & 255;
	listening = (save_code >> 8) & 1;
	shadow_exist = (save_code >> 9) & 1;
	rewind_shadow = (save_code >> 10) & 1;

	std::ifstream file(FILE_PATH, std::ios::binary | std::ios::in);
	if (!file.is_open())
		throw std::logic_error("open save file failed :" + FILE_PATH.string());
	file.seekg(0, std::ios::end);
	size_t file_size = file.tellg();
	file.seekg(file_size -(sizeof(size_t) + sizeof(bool) + SNAP_FILE_SIZE), std::ios::beg);
	//file.seekg(-static_cast<int>(sizeof(list_ptr)), std::ios::end);
	file.read(reinterpret_cast<char*>(&list_ptr), sizeof(list_ptr));
	file.close();

	/*if (gen_shadow_count > 0)
	{*/
		asyncLoadRecord();
		load_task.wait();
		if (listening || shadow_exist)
		{
			record_list = std::move(all_shadow_record.back());
			all_shadow_record.pop_back();
			if (gen_shadow_count > 1)
			{
				asyncLoadRecord();
			}
		}
	/*}*/

	if (shadow_exist)
	{
		auto shadow = new Shadow(coord.x, coord.y, save_manager, [this](float& pvc, float& phc, unsigned short& pec, unsigned int& ic)
			{this->getRecord(pvc, phc, pec, ic); });
		shadow_postal_code = shadow->getPostalCode();
	}
}

void ShadowGenerator::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (listening)
	{
		noise_shader.setUniform("time", timer.getEscapeTime());
		states.shader = &noise_shader;
	}
	target.draw(this->sprite, states);
}