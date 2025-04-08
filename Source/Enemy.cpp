#include "Enemy.h"

Enemy::Enemy(int x, int y, ObjectManagement* i_enemy_manager)
	: Object(x, y)
{
	i_enemy_manager->pushBackEnemy(this);
	is_dead = false;

	death_counter = ENEMY_DEATH_REMAIN_TIME;
}

Enemy::Enemy(int x, int y, ObjectManagement* i_enemy_manager, unsigned int i_postal_code)
	: Object(x, y, i_postal_code)
{
	i_enemy_manager->pushBackEnemy(this);
	is_dead = false;

	death_counter = ENEMY_DEATH_REMAIN_TIME;
}

const bool Enemy::getIsDead() const
{
	return is_dead;
}

void Enemy::setIsDead(bool i_is_dead)
{
	if (i_is_dead)
		die();
	else
		is_dead = false;
}