#pragma once

#include "Config.h"
#include "tinyxml2.h"
#include "MapCollision.h"
#include "Robot.h"
#include "ObjectManagement.h"
#include "Spark.h"
#include "MapNote.h"
#include "MovableWall.h"
#include "NPC.h"
#include "Receptor.h"
#include "Drawbar.h"
#include "PressurePlate.h"
#include "File.h"
#include "LaserGun.h"
#include "Helmet.h"
#include "RobotFactory.h"
#include "ShadowGenerator.h"
#include "Receptor.h"

#include <vector>
#include <sstream>

class TileMap : public sf::Drawable, public sf::Transformable
{
    //变量
private:
    struct ObjectInfo
    {
        std::array<int, 2> coord;
        sf::Color color;  
        unsigned char flip;
    };

    struct SourceInfo
    {
        int firstgid;
        std::string source;
    };

    struct VertexArrayInfo
    {
        sf::VertexArray v_array;
        int texture_id;
    };

    std::vector<sf::Texture> m_tileset;
    int m_tileWidth, m_tileHeight, m_mapWidth, m_mapHeight;
    std::vector<VertexArrayInfo> m_layers; // 存储所有图层的顶点数组

    std::vector<ObjectInfo> object_spawn_info;

    short left, right, top, bottom;

    //函数
public:
    TileMap();

    //加载地图
    bool loadFromFile(const std::string& tmxFile, ObjectManagement* i_enemy_manager);

private:
    bool parseTSX(const std::string& tsxFile, std::string& tilesetImage);
    bool parseTMX(const std::string& tmxFile, std::vector<std::vector<uint32_t>>& layersData, std::vector<SourceInfo>& tsx_source);

    //渲染地图
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    void flipHorizontally(sf::Vertex* quad);
    void flipVertiaclly(sf::Vertex* quad);
    void flipDiagonally(sf::Vertex* quad);  //顺时针旋转90度

public:
    //传出
    const unsigned short getMapWidthBlocks();
    const unsigned short getMapHeigthBlocks();

    //生成实体
    void spawnObject(ObjectManagement* i_enemy_manager, const std::string& map_name, MapNote::Note& map_note, sf::FloatRect& final, Player* player);
};