#include "TileMap.h"

TileMap::TileMap()
{
    left = 0;
    right = SCREEN_WIDTH;
    top = 0;
    bottom = SCREEN_WIDTH;
}

// 从 TMX 文件加载地图数据并解析图层
bool TileMap::loadFromFile(const std::string& tmxFile, ObjectManagement* i_enemy_manager)
{
    auto full_dir = EXE_DIR / "Resources" / "Maps" / (tmxFile + ".tmx");

    std::vector<std::vector<uint32_t>> layersData;
    std::string tilesetImage;

    // 解析 TMX 文件获取地图数据（所有图层）
    std::vector<SourceInfo> tsx_source;
    if (!parseTMX(full_dir.string(), layersData, tsx_source))
    {
        std::string str =  "Failed to parse TMX file: " + full_dir.string();
        std::cerr << str << std::endl;
        throw std::logic_error(str);
        return false;
    }

    //设置碰撞地图大小
    MapCollision::clear();
    MapCollision::setMapSize(m_mapWidth, m_mapHeight);

    //解析TSX文件获取纹理
    for(const auto& tsx : tsx_source)
    {
        full_dir = EXE_DIR / "Resources" / "Maps" / tsx.source;
        if (!parseTSX(full_dir.string(), tilesetImage))
        {
            std::string str = "Failed to parse TSX file: " + full_dir.string();
            std::cerr << str << std::endl;
            throw std::logic_error(str);
            return false;
        }

        // 加载纹理
        full_dir = EXE_DIR / "Resources" / "Maps" / tilesetImage;
        sf::Texture tileset;
        if (!tileset.loadFromFile(full_dir.string()))
        {
            std::string str = "Failed to load texture: " + full_dir.string();
            std::cerr << str << std::endl;
            throw std::logic_error(str);
            return false;
        }
        m_tileset.push_back(tileset);
    }

    // 清空旧的图层数据
    m_layers.clear();

    int flag = 0;
    // 处理每个图层的数据
    for (const auto& tileData : layersData)
    {
        for (int i = 0; i < tsx_source.size(); i++)
        {
            auto& tsx = tsx_source[i];
            auto& tileset_texture = m_tileset[i];
            bool is_base_set = tsx.source == "ExtendedMapSet.tsx" || tsx.source == "DefaultMapSet.tsx";

            int next_first_gid = -1;
            if (i < tsx_source.size() - 1)
                next_first_gid = tsx_source[i + 1].firstgid;
            bool exist_next_tsx = false;
            bool is_empty = true;
            sf::VertexArray vertices;
            vertices.setPrimitiveType(sf::Quads);
            vertices.resize(m_mapWidth * m_mapHeight * 4);

            // 填充顶点数组
            for (int y = 0; y < m_mapHeight; y++)
            {
                for (int x = 0; x < m_mapWidth; x++)
                {
                    uint32_t rawTileData = tileData[y * m_mapWidth + x];
                    int tileNumber = rawTileData & 0x1FFFFFFF; // 获取图块 ID（去除标志位）
                    tileNumber -= 1; // Tiled ID 从 1 开始，SFML 需要 0-based
                    int textureTileNumber = tileNumber - (tsx.firstgid - 1);    //得到去除纹理标志的number
                    if (next_first_gid != -1 && tileNumber >= next_first_gid)
                        exist_next_tsx = true;

                    if (textureTileNumber < 0)
                        continue; // 忽略空白图块
                    if (next_first_gid != -1 && tileNumber >= next_first_gid - 1)
                        continue;
                    is_empty = false;

                    //将第一个图层视为Wall层
                    if (tileData == layersData[0])
                    {
                        //设置碰撞
                        if (is_base_set && (textureTileNumber == 9 - 1 || textureTileNumber == 10 - 1)) //单独设置地刺的碰撞
                        {
                            MapCollision::setCell(x, y, Thorn);
                        }
                        else
                        {
                            MapCollision::setCell(x, y, Wall);
                        }
                    }

                    // 计算图块在纹理中的位置
                    int tu = textureTileNumber % (tileset_texture.getSize().x / m_tileWidth);
                    int tv = textureTileNumber / (tileset_texture.getSize().x / m_tileWidth);

                    // 判断翻转和旋转标志
                    bool index1 = (rawTileData & 0x80000000) != 0; //左右翻转
                    bool index2 = (rawTileData & 0x40000000) != 0; //上下翻转
                    bool index3 = (rawTileData & 0x20000000) != 0; //旋转

                    /*
                    编码规则如下：
                    fff：不进行任何操作
                    fft：先左右翻转，再顺时针旋转270度
                    ftf：上下翻转
                    ftt：顺时针旋转270度
                    tff：左右翻转
                    tft：顺时针旋转90度
                    ttf：旋转180度（等同于上下翻转后左右翻转）
                    ttt：先左右翻转，再顺时针旋转90度
                    */
                    short flip_case = (index1 << 2) + (index2 << 1) + index3;

                    //将第二个图层视为Object层，记录信息
                    if (flag == 1)
                    {
                        // 将纹理复制到一个Image对象中
                        sf::Image tilesetImage = tileset_texture.copyToImage();

                        // 计算图块左上角点在纹理中的坐标
                        int tilePixelX = tu * m_tileWidth;
                        int tilePixelY = tv * m_tileHeight;

                        ObjectInfo info;
                        info.coord = { x * CELL_SIZE, y * CELL_SIZE };
                        // 获取该图块左上角的像素颜色
                        info.color = tilesetImage.getPixel(tilePixelX, tilePixelY);
                        info.flip = flip_case;

                        object_spawn_info.push_back(info);
                    }
                    //Object层的内容不会绘制
                    else
                    {
                        sf::Vertex* quad = &vertices[(x + y * m_mapWidth) * 4];

                        // 设置顶点位置
                        quad[0].position = sf::Vector2f(x * m_tileWidth, y * m_tileHeight);
                        quad[1].position = sf::Vector2f((x + 1) * m_tileWidth, y * m_tileHeight);
                        quad[2].position = sf::Vector2f((x + 1) * m_tileWidth, (y + 1) * m_tileHeight);
                        quad[3].position = sf::Vector2f(x * m_tileWidth, (y + 1) * m_tileHeight);

                        //根据翻转和旋转标志调整顶点顺序
                        switch (flip_case)
                        {
                        case 0:
                            break;
                        case 1:
                            flipHorizontally(quad);
                            flipDiagonally(quad);
                            flipDiagonally(quad);
                            flipDiagonally(quad);
                            break;
                        case (1 << 1):
                            flipVertiaclly(quad);
                            break;
                        case (1 << 1) + 1:
                            flipDiagonally(quad);
                            break;
                        case (1 << 2):
                            flipHorizontally(quad);
                            break;
                        case (1 << 2) + 1:
                            flipDiagonally(quad);
                            flipDiagonally(quad);
                            flipDiagonally(quad);
                            break;
                        case (1 << 2) + (1 << 1) :
                            flipHorizontally(quad);
                            flipVertiaclly(quad);
                            break;
                        case (1 << 2) + (1 << 1) + 1:
                            flipHorizontally(quad);
                            flipDiagonally(quad);
                            break;
                        default:
                            break;
                        }


                        // 设置纹理坐标
                        quad[0].texCoords = sf::Vector2f(tu * m_tileWidth, tv * m_tileHeight);
                        quad[1].texCoords = sf::Vector2f((tu + 1) * m_tileWidth, tv * m_tileHeight);
                        quad[2].texCoords = sf::Vector2f((tu + 1) * m_tileWidth, (tv + 1) * m_tileHeight);
                        quad[3].texCoords = sf::Vector2f(tu * m_tileWidth, (tv + 1) * m_tileHeight);
                    }
                }
            }

            if (!is_empty && flag != 1)
                m_layers.push_back({ vertices, i });

            if (!exist_next_tsx)
                break;
        }
        flag++;
    }

    return true;
}

// 解析 TMX 文件以获取地图宽度、高度、瓦片宽度、高度以及图层数据
bool TileMap::parseTMX(const std::string& tmxFile, std::vector<std::vector<uint32_t>>& layersData, std::vector<SourceInfo>& tsx_source)
{
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(tmxFile.c_str()) != tinyxml2::XML_SUCCESS)
    {
        std::string str = "failed to parse tmx: " + tmxFile;
        std::cerr << str << std::endl;
        throw std::logic_error(str);
        return false;
    }

    auto mapElement = doc.FirstChildElement("map");
    if (!mapElement) return false;

    //获取.tsx文件
    //获取所有<tileset>元素
    for (auto pTileset = mapElement->FirstChildElement("tileset"); 
        pTileset != nullptr;
        pTileset = pTileset->NextSiblingElement("tileset"))
    {
        //获取source和firstgid
        const char* source = pTileset->Attribute("source");
        int firstgid = std::stoi(pTileset->Attribute("firstgid"));
        if (source == nullptr) 
            source = "DefaultMapSet.tsx";
        tsx_source.push_back(SourceInfo{ firstgid, source });
    }

    mapElement->QueryIntAttribute("width", &m_mapWidth);
    mapElement->QueryIntAttribute("height", &m_mapHeight);
    mapElement->QueryIntAttribute("tilewidth", &m_tileWidth);
    mapElement->QueryIntAttribute("tileheight", &m_tileHeight);

    // 解析所有的 layer
    for (auto layerElement = mapElement->FirstChildElement("layer");
        layerElement != nullptr;
        layerElement = layerElement->NextSiblingElement("layer"))
    {
        auto dataElement = layerElement->FirstChildElement("data");
        if (!dataElement) return false;

        const char* tileText = dataElement->GetText();
        if (!tileText) return false;

        std::vector<uint32_t> tileData;
        std::istringstream stream(tileText);
        std::string tile;
        while (std::getline(stream, tile, ','))
        {
            tileData.push_back(static_cast<uint32_t>(std::stoul(tile)));
        }

        layersData.push_back(std::move(tileData));
    }

    return true;
}

// 解析 TSX 文件以获取纹理图像的文件名
bool TileMap::parseTSX(const std::string& tsxFile, std::string& tilesetImage)
{
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(tsxFile.c_str()) != tinyxml2::XML_SUCCESS)
    {
        std::string str = "failed to parse tsx " + tsxFile;
        std::cerr << str << std::endl;
        throw std::logic_error(str);
        return false;
    }

    auto tilesetElement = doc.FirstChildElement("tileset");
    if (!tilesetElement) 
        return false;

    auto imageElement = tilesetElement->FirstChildElement("image");
    if (!imageElement) 
        return false;

    const char* source = imageElement->Attribute("source");
    if (!source) 
        return false;

    tilesetImage = source;
    return true;
}

// 绘制地图到指定的渲染目标
void TileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.blendMode = sf::BlendAlpha;

    // 计算可视范围的图块索引
    int startX = std::max((view.getCenter().x - view.getSize().x * LOADING_FACTOR)  / m_tileWidth, 0.f);
    int endX = std::min((view.getCenter().x + view.getSize().x * LOADING_FACTOR) / m_tileWidth, static_cast<float>(m_mapWidth));
    int startY = std::max((view.getCenter().y - view.getSize().y * LOADING_FACTOR) / m_tileWidth, 0.f);
    int endY = std::min((view.getCenter().y + view.getSize().y * LOADING_FACTOR) / m_tileWidth, static_cast<float>(m_mapHeight));

    for (const auto& layer : m_layers) 
    {
        states.texture = &m_tileset[layer.texture_id];

        sf::VertexArray visibleVertices;
        visibleVertices.setPrimitiveType(sf::Quads);

        for (int y = startY; y < endY; ++y)
        {
            for (int x = startX; x < endX; ++x) 
            {
                int index = (x + y * m_mapWidth) * 4;
                visibleVertices.append(layer.v_array[index]);
                visibleVertices.append(layer.v_array[index + 1]);
                visibleVertices.append(layer.v_array[index + 2]);
                visibleVertices.append(layer.v_array[index + 3]);
            }
        }

        target.draw(visibleVertices, states);
    }
}

void TileMap::flipHorizontally(sf::Vertex* quad)
{
    std::swap(quad[0].position.x, quad[1].position.x);
    std::swap(quad[2].position.x, quad[3].position.x);
    std::swap(quad[0].texCoords.x, quad[1].texCoords.x);
    std::swap(quad[2].texCoords.x, quad[3].texCoords.x);
}

void TileMap::flipVertiaclly(sf::Vertex* quad)
{
    std::swap(quad[0].position.y, quad[3].position.y);
    std::swap(quad[1].position.y, quad[2].position.y);
    std::swap(quad[0].texCoords.y, quad[3].texCoords.y);
    std::swap(quad[1].texCoords.y, quad[2].texCoords.y);
}

void TileMap::flipDiagonally(sf::Vertex* quad)
{
    std::swap(quad[2].position, quad[3].position);
    std::swap(quad[1].position, quad[2].position);
    std::swap(quad[0].position, quad[1].position);

    std::swap(quad[2].texCoords, quad[3].texCoords);
    std::swap(quad[1].texCoords, quad[2].texCoords);
    std::swap(quad[0].texCoords, quad[1].texCoords);
}

void TileMap::spawnObject(ObjectManagement* i_enemy_manager, const std::string& map_name, MapNote::Note& map_note, sf::FloatRect& final, Player* player)
{
    i_enemy_manager->reset();
    
    std::list<Receptor*> receptors;

    //尝试读取地图注释
    auto file_dir = EXE_DIR / "Resources" / "Maps" / (map_name + ".json");
    if (std::filesystem::exists(file_dir.string()))
    {
        //读取地图注释
        MapNote::parseMapNote(file_dir.string(), map_note);

        //生成movable wall
        for (const auto& i_movable_wall : map_note.movable_walls)
        {
            MovableWall* temp = new MovableWall(i_movable_wall.x * CELL_SIZE, i_movable_wall.y * CELL_SIZE, i_movable_wall.speed, i_movable_wall.length, i_enemy_manager);
            temp->init(i_movable_wall.structure_direction, i_movable_wall.move_direction, i_movable_wall.move_distance, i_movable_wall.set_can_be_affected_by_time_control);
        }

        //生成NPC
        for (const auto& i_npc : map_note.npcs)
        {
            new NPC(i_npc, i_enemy_manager);
        }

        //生成Receptor
        for (const auto& receptor : map_note.receptors_and_effectors)
        {
            Receptor* r;
            if (receptor.receptor.type == MapNote::ReceptorType::drawbar)
            {
                r = new Drawbar(receptor, i_enemy_manager);
            }
            else if (receptor.receptor.type == MapNote::ReceptorType::pressure_plate)
            {
                r = new PressurePlate(receptor, i_enemy_manager);
            }
            receptors.push_back(r);
        }
    }

    std::vector<std::array<int, 2>> final_coord;  //将会选取第一个和最后一个张成的矩形

    std::list<ShadowGenerator*> generators;

    for (auto& info : object_spawn_info)
    {
        //根据颜色生成实体
        //生成蘑菇怪
        if (info.color == sf::Color(102, 56, 49))
        {
            new Robot(info.coord[0], info.coord[1], i_enemy_manager);
        }
        else if (info.color == sf::Color(246, 129, 33))
        {
            auto temp = new Robot(info.coord[0], info.coord[1], i_enemy_manager);
            temp->setCanBeAffectedByTimeControl(false);
        }
        //设置玩家重生点
        else if (info.color == sf::Color(153, 228, 80))
        {
            player_respwan_point = { info.coord[0], info.coord[1] };
        }
        //生成金币
        else if (info.color == sf::Color(251, 242, 54))
        {
            new Spark(info.coord[0], info.coord[1], i_enemy_manager);
        }
        //设置终点
        else if (info.color == sf::Color(23, 179, 75))
        {
            //偏移一些保证与旗杆对齐
            //new Flag(info.coord[0] + 1, info.coord[1] + 8, i_enemy_manager);
            final_coord.push_back(info.coord);
        }
        else if (info.color == sf::Color(230, 208, 161))
        {
            new LaserGun(info.coord[0] + CELL_SIZE / 2, info.coord[1] + CELL_SIZE / 2, i_enemy_manager, info.flip);
        }
        else if (info.color == sf::Color(48, 191, 179))
        {
            auto lg = new LaserGun(info.coord[0] + CELL_SIZE / 2, info.coord[1] + CELL_SIZE / 2, i_enemy_manager, info.flip);
            lg->setCanBeAffectedByTimeControl(false);
        }
        else if (info.color == sf::Color(202, 219, 251))
        {
            new Helmet(info.coord[0], info.coord[1], i_enemy_manager);
        }
        else if (info.color == sf::Color(115, 255, 173))
        {
            new RobotFactory(info.coord[0] + 32, info.coord[1] + 17, i_enemy_manager, info.flip);
        }
        else if (info.color == sf::Color(255, 133, 115))
        {
            auto rf = new RobotFactory(info.coord[0] + 32, info.coord[1] + 17, i_enemy_manager, info.flip);
            rf->setCanBeAffectedByTimeControl(false);
        }
        else if (info.color == sf::Color(33, 133, 166))
        {
            auto sg = new ShadowGenerator(info.coord[0], info.coord[1] + CELL_SIZE / 2, i_enemy_manager);
            sg->registerListener(std::bind(&Player::getStates, player, 
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            generators.push_back(sg);
        }
    }

    //设置张成的矩形
    if (final_coord.size() > 0)
    {
        final.left = final_coord.at(0).at(0);
        final.top = final_coord.at(0).at(1);
        final.width = abs(final_coord.at(0).at(0) - final_coord.at(final_coord.size() - 1).at(0)) + CELL_SIZE;
        final.height = abs(final_coord.at(0).at(1) - final_coord.at(final_coord.size() - 1).at(1)) + CELL_SIZE;
    }

    if (generators.size() > 0 && receptors.size() > 0)
    {
        for (const auto& g : generators)
        {
            for (auto r : receptors)
            {
                g->registerListener(std::bind(&Receptor::getStateByPostalCode, r, std::placeholders::_1));
            }
        }
    }
}

const unsigned short TileMap::getMapWidthBlocks()
{
    return this->m_mapWidth;
}

const unsigned short TileMap::getMapHeigthBlocks()
{
    return this->m_mapHeight;
}