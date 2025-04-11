#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <string>
#include <sstream>
#include <filesystem>
#include <random>
#include <stdexcept>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>

//游戏参数
constexpr unsigned short SCREEN_WIDTH = 800;
constexpr unsigned short SCREEN_HEIGHT = 450;
constexpr unsigned char CELL_SIZE = 32;
constexpr float TRACK_SPEED = 0.12;
constexpr float LOADING_FACTOR = 0.6;
constexpr float VIEW_TRACKING_EDGE = 12.5;
constexpr float VIEW_TRACKING_EDGE_VERTICAL = 7.5;
constexpr unsigned char FPS = 60;
constexpr unsigned char MESSAGE_QUEUE_CAPACITY = 10;
constexpr unsigned char INTERFACE_KEYBOARD_SELECT_DELAY = 7;
constexpr unsigned char MAP_DESCRIPTION_SHOW_TIME = 120;
constexpr unsigned char MAP_BEGIN_AND_END_TIME = 120;	//地图出现时淡入淡出的持续时间
constexpr unsigned char RECEPTOR_DELAY = 20;

//角色速度
constexpr float PLAYER_SPEED = 3.5;
constexpr float PLAYER_JUMP_SPEED = -10;
constexpr float PLAYER_ACCELERATION = 0.15;
constexpr float GOOMBA_SPEED = 2.5;
constexpr float PLAYER_BOUNCE_SPEED = -14;

//物理
constexpr float GRAVITY = 0.6;
constexpr float RESISTANCE = 0.3;
constexpr float MAX_FALL_SPEED = 16;

//机制
constexpr unsigned char JUMP_TIMER = 15;
constexpr unsigned char DEATH_COUNTER = 3;
constexpr unsigned short MAX_RECORD_TIME = 15;	//最多记录多少秒
constexpr unsigned char RECORD_DENSITY = 10; //每隔多少帧记录一次
constexpr float SAVE_AND_LOAD_RATIO = 3;
constexpr unsigned short ENEMY_DEATH_REMAIN_TIME = 180;	//敌人死亡后尸体保留的时长
constexpr unsigned char LONGEST_AUTO_REWIND_SECOND = 5;
constexpr unsigned char MAX_CONTINUE_JUMP_NUM = 2;

//动画
constexpr unsigned char ANIMATION_SPEED = 7;
constexpr unsigned char HELMET_GRAIN_EFFECT_DENSITY = 15;
constexpr unsigned char CAMERA_SHAKE_TIME = 5;

//调试
#define SHOW_CONSOLE false
#define DRAW_HIT_BOX false
#define DRAW_ZONE false
#define ALLOCATION_INDICATOR false
constexpr float VIEW_SIZE = 1;
#include "Debug.h"

enum Cell
{
	Empty = 0,
	Wall,
	Thorn
};

enum Direction
{
	left = 0,
	right,
	up,
	down,
	null
};

extern short map_width_brick;
extern short map_height_brick;
extern sf::Vector2i player_respwan_point;
extern unsigned short spark_num;
extern unsigned int points;

extern unsigned short ending;	//决定进入的结局
extern unsigned short switch_to_next_scene;	//对话结束后直接进入下一关

extern unsigned short ending_has_achieved;	//记录达成结局的情况

extern unsigned char minute, second, fps_counter;	//计时器

extern const std::filesystem::path EXE_DIR;

extern std::mt19937 gen;

extern sf::View view;

#if SHOW_CONSOLE
extern bool cheating_mode;
#endif

#if ALLOCATION_INDICATOR
#define new new(__FILE__, __LINE__)
#endif