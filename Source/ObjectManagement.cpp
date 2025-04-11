#include "ObjectManagement.h"
#include "Enemy.h"
#include "Player.h"

ObjectManagement::ObjectManagement()
{
    loading_range.left = 0;
    loading_range.top = 0;
    loading_range.width = view.getSize().x * 2 * LOADING_FACTOR;
    loading_range.height = view.getSize().y * 2 * LOADING_FACTOR;

    player_death_counter = DEATH_COUNTER;

    can_interact = false;
}

ObjectManagement::~ObjectManagement()
{
    reset();
}

void ObjectManagement::pushBackEnemy(Enemy* i_enemy)
{
    enemy_list.push_back(i_enemy);
}

void ObjectManagement::pushBackObject(Object* i_object)
{
    object_list.push_back(i_object);
}

void ObjectManagement::registerPlayer(Player* i_player)
{
    player = i_player;
}

void ObjectManagement::reset()
{
    for (auto enemy = enemy_list.begin(); enemy != enemy_list.end(); enemy++)
    {
        delete* enemy;
    }
    enemy_list.clear();

    for (auto object = object_list.begin(); object != object_list.end(); object++)
    {
        delete* object;
    }
    object_list.clear();
}

bool ObjectManagement::eraseEnemy(Enemy* i_enemy)
{
    for (auto enemy = enemy_list.begin(); enemy != enemy_list.end(); enemy++)
    {
        if (*enemy == i_enemy)
        {
            enemy_list.erase(enemy);
            if (i_enemy != nullptr)
                delete i_enemy;
            return true;
        }
    }
    return false;
}

bool ObjectManagement::eraseObject(Object* i_object)
{
    for (auto object = object_list.begin(); object != object_list.end(); object++)
    {
        if (*object == i_object)
        {
            object_list.erase(object);
            if (i_object != nullptr)
                delete i_object;
            return true;
        }
    }
    return false;
}

void ObjectManagement::setLoadingRange(sf::Vector2f pp)
{
    //确定模拟范围
    loading_range.width = view.getSize().x * 2 * LOADING_FACTOR;
    loading_range.height = view.getSize().y * 2 * LOADING_FACTOR;
    loading_range.left = static_cast<float>(view.getCenter().x) - view.getSize().x * LOADING_FACTOR;
    loading_range.top = static_cast<float>(view.getCenter().y) - view.getSize().y * LOADING_FACTOR;
}

void ObjectManagement::update()
{
    can_interact = false;

    //更新玩家
    player->update();

    sf::Vector2f player_coord = player->getCoord();

    //设置玩家碰撞箱
    sf::FloatRect player_hit_box = *player->getHitBox();

    //逐个更新敌人
    bool player_hit_by_robot = false;
    for (auto enemy = enemy_list.begin(); enemy != enemy_list.end(); enemy++)
    {
        if (*enemy != nullptr)
        {
            sf::Vector2f enemy_position = (*enemy)->getCoord();
            //不再检测加载范围
            /*if (MapCollision::object_collision((*enemy)->getHitBox(), &loading_range))
            {*/
                //移除不加载的敌人
                if ((*enemy)->getShouldUpdate() == false)
                {
                    delete* enemy;
                    enemy = enemy_list.erase(enemy);
                    // 在这里检查避免解引用无效迭代器
                    if (enemy == enemy_list.end())
                        break;
                }
                
                if ((*enemy)->getIsDead() == false)
                {
                    //碰撞检测
                    for (auto a_enemy = enemy; a_enemy != enemy_list.end(); a_enemy++)
                    {
                        //忽略自身
                        if (*a_enemy == *enemy)
                            continue;

                        //忽略死的
                        if ((*a_enemy)->getIsDead() == true)
                            continue;

                        if (MapCollision::object_collision((*enemy)->getHitBox(), (*a_enemy)->getHitBox()))
                        {
                            //设置为蘑菇怪之间相撞
                            //并给出碰撞的方向
                            //此处只区分左右
                            if ((*a_enemy)->getCoord().x - enemy_position.x > 0)
                            {
                                if ((*enemy)->getSendMessage())
                                    MessageQueue::send((*enemy)->getPostalCode(), (*a_enemy)->getPostalCode(), "collision from left");
                                if ((*a_enemy)->getSendMessage())
                                    MessageQueue::send((*a_enemy)->getPostalCode(), (*enemy)->getPostalCode(), "collision from right");
                            }
                            else
                            {
                                if ((*enemy)->getSendMessage())
                                    MessageQueue::send((*enemy)->getPostalCode(), (*a_enemy)->getPostalCode(), "collision from right");
                                if ((*a_enemy)->getSendMessage())
                                    MessageQueue::send((*a_enemy)->getPostalCode(), (*enemy)->getPostalCode(), "collision from left");
                            }
                        }
                    }

                    //检测与玩家相撞
                    if (player->getHasFailed() == false)
                    {
                            if (MapCollision::object_collision(&player_hit_box, (*enemy)->getHitBox()))
                            {
                                //判断是玩家踩死蘑菇怪还是蘑菇怪撞死玩家
                                if (player->getVerticalSpeed() > 0 && MessageQueue::computeInterclassicCode((*enemy)->getPostalCode()) == MessageQueue::robot_header)
                                {
                                    if (player->getSendMessage())
                                        MessageQueue::send(MessageQueue::player_code, (*enemy)->getPostalCode(), "defeated by player");
                                    if ((*enemy)->getSendMessage())
                                        MessageQueue::send((*enemy)->getPostalCode(), MessageQueue::player_code, "player bounce");
                                    MessageQueue::send(MessageQueue::player_code, MessageQueue::scene_code, "camera shake");
                                }
                                else
                                {
                                    player_hit_by_robot = true;
                                }
                            }
                    }
                }

                (*enemy)->update();
            //}
        }
        else
        {
            enemy_list.erase(enemy);
        }
    }

    if (player_hit_by_robot)
    {
        player_death_counter--;
        if (player_death_counter == 0)
            MessageQueue::send(static_cast<unsigned int>(MessageQueue::robot_header << 26), MessageQueue::player_code, "player hitted by robot");
    }
    else
    {
        player_death_counter = DEATH_COUNTER;
    }

    player->setReferenceSystem(0, 0);

    //逐个更新实体
    for (int i = 0; i < object_list.size(); i++)
    {
        auto& object = object_list.at(i);

        if (object == nullptr)
        {
            eraseObject(object);
            continue;
        }

        if (object->getShouldUpdate() == true)
        {
            sf::Vector2f object_position = object->getCoord();
            //检测与玩家相撞
            if (player->getHasFailed() == false)
            {
                if (MessageQueue::computeInterclassicCode(object->getPostalCode()) == MessageQueue::movable_wall_header)
                {
                    if (MapCollision::object_collision(&player_hit_box, object->getHitBox()))
                    {
                        if (player->getCoord().y <= object->getHitBox()->top + 1)
                            player->setReferenceSystem(object->getHorizontalSpeed(), object->getVerticalSpeed(), true, object->getHitBox()->top + 1);
                        else
                            player->setReferenceSystem(object->getHorizontalSpeed(), object->getVerticalSpeed());
                        /*std::cout << '1' << '\n';*/
                    }
                    

                    //检测敌人
                    for (const auto& enemy : enemy_list)
                    {
                        if (MapCollision::object_collision(enemy->getHitBox(), object->getHitBox()) && enemy->getCoord().y <= object->getHitBox()->top + 1)
                        {
                            if (enemy->getCoord().y <= object->getHitBox()->top + 1)
                                enemy->setReferenceSystem(object->getHorizontalSpeed(), object->getVerticalSpeed(), true, object->getHitBox()->top + 1);
                            else
                                enemy->setReferenceSystem(object->getHorizontalSpeed(), object->getVerticalSpeed());
                        }
                        else
                        {
                            enemy->setReferenceSystem(0, 0, false, 0.f);
                        }
                    }
                }
                else
                {
                    if (MapCollision::object_collision(&player_hit_box, object->getHitBox()))
                    {
                        if (object->getSendMessage())
                            MessageQueue::send(MessageQueue::player_code, object->getPostalCode(), "hitted by player");
                        if (object->getInteractivable())
                            can_interact = true;
                    }

                    if (MessageQueue::computeInterclassicCode(object->getPostalCode()) == MessageQueue::receptor_header)
                    {
                        //额外检测与敌人的碰撞
                        for (const auto& enemy : enemy_list)
                        {
                            if (MapCollision::object_collision(enemy->getHitBox(), object->getHitBox()))
                            {
                                MessageQueue::send(enemy->getPostalCode(), object->getPostalCode(), "hitted by enemy");
                                break;
                            }
                        }
                    }
                }
            }
            object->update();
        }
        else
        {
            //删除不加载的实体
            eraseObject(object);
        }
    }
}

void ObjectManagement::updateThoseWhoCouldntBeAffectedByTimeControl()
{
    can_interact = false;

    //更新玩家
    if (player->canBeAffectedByTimeControl() == false)
        player->update();

    sf::Vector2f player_coord = player->getCoord();

    //设置玩家碰撞箱
    sf::FloatRect player_hit_box = *player->getHitBox();

    //逐个更新敌人
    bool player_hit_by_robot = false;
    for (auto enemy = enemy_list.begin(); enemy != enemy_list.end(); enemy++)
    {
        if ((*enemy)->canBeAffectedByTimeControl() == true)
        {
            //时间倒流时，激光可以打死(受到时间倒流影响的)小怪
            if (MessageQueue::computeInterclassicCode((*enemy)->getPostalCode()) == MessageQueue::laser_header)
            {
                for (auto b_enemy = enemy_list.begin(); b_enemy != enemy_list.end(); b_enemy++)
                {
                    if (MapCollision::object_collision((*enemy)->getHitBox(), (*b_enemy)->getHitBox())
                        && MessageQueue::computeInterclassicCode((*b_enemy)->getPostalCode()) == MessageQueue::robot_header)
                        //&& (*b_enemy)->canBeAffectedByTimeControl() == true)
                    {
                        (*b_enemy)->setIsDead(true);
                        (*b_enemy)->setCanBeAffectedByTimeControl(false);
                    }
                }
            }
            continue;
        }

        if (*enemy != nullptr)
        {
            sf::Vector2f enemy_position = (*enemy)->getCoord();
            //检测加载范围
            /*if (MapCollision::object_collision((*enemy)->getHitBox(), &loading_range))
            {*/
                //移除不加载的敌人
                if ((*enemy)->getShouldUpdate() == false)
                {
                    delete* enemy;
                    enemy = enemy_list.erase(enemy);
                    // 在这里检查避免解引用无效迭代器
                    if (enemy == enemy_list.end())
                        break;
                }

                if ((*enemy)->getIsDead() == false)
                {
                    //碰撞检测
                    for (auto a_enemy = enemy; a_enemy != enemy_list.end(); a_enemy++)
                    {
                        //忽略自身
                        if (*a_enemy == *enemy)
                            continue;

                        //忽略死的
                        if ((*a_enemy)->getIsDead() || (*a_enemy)->getShouldUpdate() == false)
                            continue;

                        if (MapCollision::object_collision((*enemy)->getHitBox(), (*a_enemy)->getHitBox()))
                        {
                            //设置为蘑菇怪之间相撞
                            //并给出碰撞的方向
                            //此处只区分左右
                            if ((*a_enemy)->getCoord().x - enemy_position.x > 0)
                            {
                                if ((*enemy)->getSendMessage())
                                    MessageQueue::send((*enemy)->getPostalCode(), (*a_enemy)->getPostalCode(), "collision from left");
                                if ((*a_enemy)->getSendMessage())
                                    MessageQueue::send((*a_enemy)->getPostalCode(), (*enemy)->getPostalCode(), "collision from right");
                            }
                            else
                            {
                                if ((*enemy)->getSendMessage())
                                    MessageQueue::send((*enemy)->getPostalCode(), (*a_enemy)->getPostalCode(), "collision from right");
                                if ((*a_enemy)->getSendMessage())
                                    MessageQueue::send((*a_enemy)->getPostalCode(), (*enemy)->getPostalCode(), "collision from left");
                            }
                        }
                    }

                    //检测与玩家相撞
                    if (player->getHasFailed() == false)
                    {
                            if (MapCollision::object_collision(&player_hit_box, (*enemy)->getHitBox()))
                            {
                                //判断是玩家踩死蘑菇怪还是蘑菇怪撞死玩家
                                if (player->getVerticalSpeed() > 0)
                                {
                                    if (player->getSendMessage())
                                        MessageQueue::send(MessageQueue::player_code, (*enemy)->getPostalCode(), "defeated by player");
                                    if ((*enemy)->getSendMessage())
                                        MessageQueue::send((*enemy)->getPostalCode(), MessageQueue::player_code, "player bounce");
                                }
                                else
                                {
                                    player_hit_by_robot = true;
                                }
                            }
                    }
                }

                (*enemy)->update();
            //}
        }
        else
        {
            enemy_list.erase(enemy);
        }
    }

    if (player_hit_by_robot)
    {
        player_death_counter--;
        if (player_death_counter == 0)
            MessageQueue::send(static_cast<unsigned int>(MessageQueue::robot_header << 26), MessageQueue::player_code, "player hitted by robot");
    }
    else
    {
        player_death_counter = DEATH_COUNTER;
    }

    player->setReferenceSystem(0, 0);

    //逐个更新实体
    for (auto& object : object_list)
    {
        if (object == nullptr)
        {
            eraseObject(object);
            continue;
        }

        if (object->canBeAffectedByTimeControl() == true)
            continue;

        if (object->getShouldUpdate() == true)
        {
            sf::Vector2f object_position = object->getCoord();
            //检测加载范围
           /* if (MapCollision::object_collision(object->getHitBox(), &loading_range))
            {*/
                //检测与玩家相撞
                if (player->getHasFailed() == false)
                {
                        if (MessageQueue::computeInterclassicCode(object->getPostalCode()) == MessageQueue::movable_wall_header)
                        {
                            if (player->canBeAffectedByTimeControl() == false)
                            {
                                if (MapCollision::object_collision(&player_hit_box, object->getHitBox()))
                                {
                                    player->setReferenceSystem(object->getHorizontalSpeed(), object->getVerticalSpeed());
                                }
                                
                            }

                            //检测敌人
                            for (const auto& enemy : enemy_list)
                            {
                                if (enemy->canBeAffectedByTimeControl() == true)
                                    continue;
                                if (MapCollision::object_collision(enemy->getHitBox(), object->getHitBox()))
                                {
                                    enemy->setReferenceSystem(object->getHorizontalSpeed(), object->getVerticalSpeed());
                                }
                                else
                                {
                                    enemy->setReferenceSystem(0, 0);
                                }
                            }
                        }
                        else
                        {
                            if (MapCollision::object_collision(&player_hit_box, object->getHitBox()))
                            {
                                if (object->getSendMessage())
                                    MessageQueue::send(MessageQueue::player_code, object->getPostalCode(), "hitted by player");
                                if (object->getInteractivable())
                                    can_interact = true;
                            }

                            if (MessageQueue::computeInterclassicCode(object->getPostalCode()) == MessageQueue::receptor_header)
                            {
                                //额外检测与敌人的碰撞
                                for (const auto& enemy : enemy_list)
                                {
                                    if (MapCollision::object_collision(enemy->getHitBox(), object->getHitBox()))
                                    {
                                        MessageQueue::send(enemy->getPostalCode(), object->getPostalCode(), "hitted by enemy");
                                        break;
                                    }
                                }
                            }
                        }
                }
                object->update();
            //}
        }
        else
        {
            //删除不加载的实体
            eraseObject(object);
            //delete object;
        }
    }
}

void ObjectManagement::draw(sf::RenderTexture& texture) const
{
    for (auto& enemy : enemy_list)
    {
        sf::Sprite* sprite = enemy->getSprite();
        if (enemy != nullptr && sprite != nullptr)
        {
            sf::Vector2f enemy_position = enemy->getCoord();
            if (MapCollision::object_collision(enemy->getHitBox(), &loading_range))
            {
                if (enemy->canBeAffectedByTimeControl())
                    enemy->draw(texture);
                /*else
                    enemy->draw(window, sf::RenderStates::Default);
                if (DRAW_HIT_BOX)
                {
                    sf::RectangleShape hit_box;
                    const sf::FloatRect* i_hit_box = enemy->getHitBox();
                    hit_box.setFillColor(sf::Color::Transparent);
                    hit_box.setOutlineColor(sf::Color::Red);
                    hit_box.setOutlineThickness(1);
                    hit_box.setSize({ i_hit_box->width, i_hit_box->height });
                    hit_box.setPosition({ i_hit_box->left, i_hit_box->top });
                    window.draw(hit_box);
                }*/
            }
        }
    }

    for (auto& object : object_list)
    {
        sf::Sprite* sprite = object->getSprite();
        if (object != nullptr && sprite != nullptr)
        {
            sf::Vector2f object_position = object->getCoord();
            if (MapCollision::object_collision(object->getHitBox(), &loading_range))
            {
                if (object->canBeAffectedByTimeControl() || MessageQueue::computeInterclassicCode(object->getPostalCode()) == MessageQueue::NPC_header 
                    || MessageQueue::computeInterclassicCode(object->getPostalCode()) == MessageQueue::shadow_header)
                    object->draw(texture);
               /* else
                    object->draw(window, sf::RenderStates::Default);

#if DRAW_HIT_BOX
                    sf::RectangleShape hit_box;
                    const sf::FloatRect* i_hit_box = object->getHitBox();
                    hit_box.setFillColor(sf::Color::Transparent);
                    hit_box.setOutlineColor(sf::Color::Red);
                    hit_box.setOutlineThickness(1);
                    hit_box.setSize({ i_hit_box->width, i_hit_box->height });
                    hit_box.setPosition({ i_hit_box->left, i_hit_box->top });
                    window.draw(hit_box);
#endif*/
            }
        }
    }

    if (player->canBeAffectedByTimeControl())
        player->draw(texture);
    /*else
        player->draw(window, sf::RenderStates::Default);

#if DRAW_HIT_BOX
        sf::RectangleShape hit_box;
        const sf::FloatRect* i_hit_box = player->getHitBox();
        hit_box.setFillColor(sf::Color::Transparent);
        hit_box.setOutlineColor(sf::Color::Red);
        hit_box.setOutlineThickness(1);
        hit_box.setSize({ i_hit_box->width, i_hit_box->height });
        hit_box.setPosition({ i_hit_box->left, i_hit_box->top });
        window.draw(hit_box);

        MapCollision::draw_hit_box(&window);
#endif*/
}

void ObjectManagement::draw(sf::RenderWindow& window, sf::RenderStates states) const
{
    for (auto& enemy : enemy_list)
    {
        sf::Sprite* sprite = enemy->getSprite();
        if (enemy != nullptr && sprite != nullptr)
        {
            sf::Vector2f enemy_position = enemy->getCoord();
            if (MapCollision::object_collision(enemy->getHitBox(), &loading_range))
            {
                if (enemy->canBeAffectedByTimeControl() == false)
                    enemy->draw(window, states);
#if DRAW_HIT_BOX
                sf::RectangleShape hit_box;
                const sf::FloatRect* i_hit_box = enemy->getHitBox();
                hit_box.setFillColor(sf::Color::Transparent);
                hit_box.setOutlineColor(sf::Color::Red);
                hit_box.setOutlineThickness(1);
                hit_box.setSize({ i_hit_box->width, i_hit_box->height });
                hit_box.setPosition({ i_hit_box->left, i_hit_box->top });
                window.draw(hit_box);
#endif
            }
        }
    }

    for (auto& object : object_list)
    {
        sf::Sprite* sprite = object->getSprite();
        if (object != nullptr && sprite != nullptr)
        {
            if (MapCollision::object_collision(object->getHitBox(), &loading_range))
            {
                if (object->canBeAffectedByTimeControl() == false && MessageQueue::computeInterclassicCode(object->getPostalCode()) != MessageQueue::NPC_header
                    && MessageQueue::computeInterclassicCode(object->getPostalCode()) != MessageQueue::shadow_header)
                    object->draw(window, states);

 #if DRAW_HIT_BOX
                     sf::RectangleShape hit_box;
                     const sf::FloatRect* i_hit_box = object->getHitBox();
                     hit_box.setFillColor(sf::Color::Transparent);
                     hit_box.setOutlineColor(sf::Color::Red);
                     hit_box.setOutlineThickness(1);
                     hit_box.setSize({ i_hit_box->width, i_hit_box->height });
                     hit_box.setPosition({ i_hit_box->left, i_hit_box->top });
                     window.draw(hit_box);
 #endif
            }
        }
    }

    if (player->canBeAffectedByTimeControl() == false)
        player->draw(window, states);

#if DRAW_HIT_BOX
        sf::RectangleShape hit_box;
        const sf::FloatRect* i_hit_box = player->getHitBox();
        hit_box.setFillColor(sf::Color::Transparent);
        hit_box.setOutlineColor(sf::Color::Red);
        hit_box.setOutlineThickness(1);
        hit_box.setSize({ i_hit_box->width, i_hit_box->height });
        hit_box.setPosition({ i_hit_box->left, i_hit_box->top });
        window.draw(hit_box);

        MapCollision::draw_hit_box(&window);
#endif
}

const std::vector<Enemy*>& ObjectManagement::getEnemyList() const
{
    return enemy_list;
}

const std::vector<Object*>& ObjectManagement::getObjectList() const
{
    return object_list;
}

const bool ObjectManagement::getCanInteract() const
{
    return can_interact;
}

const bool ObjectManagement::checkIsPlayerSafe() const
{
    if (player->getHasFailed())
        return false;

    //检测是否站在地面上
    float cell_x1, cell_x2, cell_y;
    cell_y = player->getCoord().y / CELL_SIZE + 1;
    cell_x1 = (player->getCoord().x - CELL_SIZE / 2);
    cell_x2 = (player->getCoord().x + CELL_SIZE / 2);
    if (MapCollision::getCell(cell_x1, cell_y) == Empty || MapCollision::getCell(cell_x2, cell_y) == Empty)
        return false;
    unsigned char collision = MapCollision::map_collision(player->getHitBox());
    /*if (!(collision & (1 << 3)))
        return false;*/
    if (collision & (1 << 2) || collision & (1 << 1) || collision & (1 << 0))
        return false;

    //检测地刺
    if (collision & (1 << 4))
        return false;

  /*  float temp = (player->getCoord().x - CELL_SIZE / 2) / static_cast<float>(CELL_SIZE);
    if (player->getCoord().x - CELL_SIZE / 2 - floor(temp) * CELL_SIZE > 1)
        return false;*/

    //检测与敌人的距离
    for (const auto& enemy : enemy_list)
    {
        if (std::abs(enemy->getCoord().x - player->getCoord().x) + std::abs(enemy->getCoord().y - player->getCoord().y) < 5 * CELL_SIZE)
            return false;
    }

    return true;
}