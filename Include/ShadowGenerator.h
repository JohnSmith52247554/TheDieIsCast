#pragma once

#include "Config.h"
#include "Object.h"
#include "ObjectManagement.h"
#include "Shadow.h"
#include "Debug.h"

#include <functional>
#include <deque>
#include <list>
#include <fstream>
#include <thread>
#include <mutex>
#include <future>

class ShadowGenerator : public Object
{
private:
	const size_t SNAP_FILE_SIZE = sizeof(float) * 2 + sizeof(unsigned short) + sizeof(unsigned int);

	std::mutex file_mutex;
	std::future<void> save_task;
	std::future<void> load_task;
	std::atomic<bool> is_saving{ false };
	std::atomic<bool> is_loading{ false };

	std::filesystem::path FILE_PATH;

	struct ShadowSnap
	{
		float player_vertical_coord, player_horizontal_coord;
		unsigned short player_eigen_code;
		unsigned int interact_postal_code;
	};

	using player_listener = std::function<void(float&, float&, unsigned short&)>;
	using interactivable_listener = std::function<void(unsigned int&)>;
	player_listener listen_player;
	std::vector<interactivable_listener> listen_interactivable;

	std::list<std::deque<ShadowSnap>> all_shadow_record;
	std::deque<ShadowSnap> record_list;

	size_t list_ptr;

	unsigned int shadow_postal_code;

	unsigned char gen_shadow_count;

	ObjectManagement* save_manager;

	bool shadow_exist;
	bool hitted, prev_hitted;
	bool listening;

	bool rewind_shadow;

	bool saved;

	mutable sf::Shader noise_shader;

	Debug::Timer timer;

public:
	ShadowGenerator(int x, int y, ObjectManagement* object_manager, bool load = false);
	ShadowGenerator(int x, int y, ObjectManagement* object_manager, bool load, unsigned int setted_postal_code);
	~ShadowGenerator();

	void registerListener(player_listener listener);
	void registerListener(interactivable_listener listener);

	virtual void update() override;

	virtual const sf::FloatRect* getHitBox() override;

	virtual const unsigned short encode() const override;
	virtual void decode(const unsigned short i_eigen_code) override;

private:
	void listen();

	virtual void react() override;

	void getRecord(float& pvc, float& phc, unsigned short& pec, unsigned int& ic);

	void asyncSaveRecord();
	void asyncLoadRecord();

public:
	virtual const unsigned short saveCode() override;
	virtual void praseSaveCode(const unsigned short save_code) override;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const override;
};
