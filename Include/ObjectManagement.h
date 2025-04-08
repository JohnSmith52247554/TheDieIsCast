#pragma once

#include "Config.h"
#include "MessageQueueSubscriber.h"

class Object;
class Enemy;
class Player;

class ObjectManagement
{
	//变量
private:
	std::vector<Enemy*> enemy_list;
	std::vector<Object*> object_list;
	sf::FloatRect loading_range;
	unsigned char player_death_counter;

	Player* player;

	bool can_interact;

	//函数
public:
	ObjectManagement();
	~ObjectManagement();

	void pushBackEnemy(Enemy* i_enemy);
	void pushBackObject(Object* i_object);
	void registerPlayer(Player* i_player);

	void reset();

	bool eraseEnemy(Enemy* i_enemy);
	bool eraseObject(Object* i_object);

	void setLoadingRange(sf::Vector2f pp);

	void update();
	void updateThoseWhoCouldntBeAffectedByTimeControl();
	void draw(sf::RenderTexture& texture) const;
	void draw(sf::RenderWindow& window, sf::RenderStates states = sf::RenderStates::Default) const;

	const std::vector<Enemy*>& getEnemyList() const;
	const std::vector<Object*>& getObjectList() const;

	const bool getCanInteract() const;

	const bool checkIsPlayerSafe() const;
};

