/*

锟斤拷戏锟斤拷锟斤拷
锟斤拷原锟斤拷锟斤拷Game迁锟狡讹拷锟斤拷

*/

#pragma once

#include "Config.h"
#include "Player.h"
#include "TileMap.h"
#include "ObjectManagement.h"
#include "Text.h"
#include "TimeManagement.h"
#include "Bar.h"
#include "Interface.h"
#include "MessageQueueSubscriber.h"
#include "Script.h"
#include "MapNote.h"
#include "File.h"

#include <fstream>
#include <chrono>

class Scene : public MessageQueueSubscriber
{
private:
	friend class Game;

	//锟斤拷锟斤拷
private:
	sf::RenderWindow* window;	//锟斤拷为锟斤拷Game锟斤拷锟斤拷
	/*sf::View* view;*/

	TileMap map;
	ObjectManagement* object_manager;
	sf::Text* text;
	TimeManagement* time_manager;
	Bar* bar;
	Interface* i_interface;
	DialogueInterface* dialogue_interface;

	Text* press_f_reminder;

	Player* player;

	sf::RectangleShape* full_screen_quad;	//锟斤拷锟斤拷全锟斤拷锟侥筹拷锟斤拷锟轿ｏ拷锟斤拷锟斤拷实锟斤拷锟剿撅拷效锟斤拷

	Script::Map script_map;	//锟斤拷锟节达拷锟斤拷锟斤拷锟斤拷锟较?1?7

	MapNote::Note map_note;

	struct DialogueTriggerZoneAndTriggerID
	{
		sf::FloatRect dialogue_trigger_zone;
		int trigger_dialouge_id;
		int trigger_id;
	};
	std::vector<DialogueTriggerZoneAndTriggerID> dialogue_trigger_zones;	//锟皆伙拷锟斤拷锟斤拷锟斤拷围

	sf::Sprite* background;	//锟斤拷锟斤拷锟斤拷锟斤拷锟接斤拷锟狡讹拷
	sf::Texture* background_texture;

	sf::RenderTexture render_texture;
	sf::Sprite screen_sprite;

	bool endOfGame;
	bool is_rewinding_time;
	bool loading_shader;
	bool game_pause;
	bool show_interface;
	bool has_player_failed;

	bool earthquake;
	const float MAX_AMPLITUDE = 9;
	const float MIN_AMPLITUDE = 7;
	const unsigned char RANDOM_QUAKE_COUNTER = 1;
	unsigned char random_quake_counter;
	unsigned char camera_shake_counter;
	bool play_quake_music;
	bool shake_direction;

	unsigned char begin_end_counter;	//锟斤拷锟节达拷锟斤拷锟斤拷锟斤拷锟斤拷始锟酵斤拷锟斤拷时锟侥碉拷锟诫淡锟斤拷效锟斤拷
	unsigned char state;	//0锟斤拷示锟斤拷始锟侥碉拷锟诫，1锟斤拷示锟斤拷戏锟斤拷锟叫ｏ拷2锟斤拷锟斤拷锟较憋拷示锟斤拷锟斤拷锟侥碉拷锟斤拷

	struct NPCinfo
	{
		short current_speaking_npc_id;
		unsigned int current_speaking_npc_postal_code;
	} npc_info;

	unsigned short auto_rewind_time_counter;

	sf::FloatRect final;	//锟截匡拷锟斤拷锟秸点，锟斤拷锟斤拷锟斤拷锟阶诧拷锟轿拷锟?1?7

	sf::Shader shader;
	sf::RenderStates states;
	sf::Shader shader2;
	sf::RenderStates states2;
	bool shader_available;

	Debug::Timer timer;

#if SHOW_CONSOLE
	Text* cheating_mode_reminder;
#endif

	unsigned char moving_camera;
	float move_camera_speed;

	bool force_rewind;

	unsigned int forward_time_counter;

	//锟斤拷锟斤拷
private:
	//锟斤拷始锟斤拷
	void initVar();
	void initWin(sf::RenderWindow* i_render_window);
	void initObject(std::string i_map_name);

	//锟斤拷锟斤拷锟接斤拷
	void setView(sf::Vector2f coord);
	void trackPlayer();
	void setViewToPlayer();
	void setViewToTheOtherSide();

	//锟斤拷锟斤拷锟绞硷拷
	virtual void react() override;

	//锟斤拷锟矫撅拷锟介触锟斤拷
	void setDialogueTrigger();

	void moveBackground();

	void setGlobalMode();

public:
	//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	Scene(Script::Map& i_script_map, sf::RenderWindow* i_render_window);
	Scene(Script::Map& i_script_map, sf::RenderWindow* i_render_window, bool load);
	~Scene();

	//锟斤拷锟斤拷锟斤拷锟斤拷

	//锟斤拷锟铰猴拷锟斤拷
	void update();

	//锟斤拷染锟斤拷锟斤拷
	void render();

	const bool canControlPlayer() const;

private:
	void save();
	void load();

	void loadRecord();
};