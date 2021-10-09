//
// Created by baifeng on 2021/10/2.
//

#ifndef SDL2_UI_DATA_H
#define SDL2_UI_DATA_H

#include "common/types.h"
#include "common/quadtree.h"
#include "obs.h"
#include <vector>
#include <string>

class TileData {
public:
    TileData();
public:
    int layer;
    int type;
};

//===================================================================
// 关卡配置

namespace Tile {
#define IC(name, value) static int const name = value
    IC(SIZE, 40);
    IC(MAP_SIZE, SIZE * 13);
    enum Type {
        NONE = 0,
        BASE = 0x100,
        BRICK,
        STEEL,
        WATERS,
        TREES,
        ICE_FLOOR,
        TYPE_END,
    };
}

namespace Tank {
    enum Type {

    };
    enum Direction {
        UP = 0,
        RIGHT,
        DOWN,
        LEFT,
        MAX,
    };
    enum Camp {
        FRIENDLY = 0x20,
        ENEMY
    };
}

typedef struct {
    Tile::Type type;
    int x;
    int y;
} AddTile;

typedef struct {
    Tank::Type type;
} AddTank;

typedef std::vector<AddTile> AddTileList;
typedef std::vector<AddTank> AddTankList;

class TileView;
class TileModel: public obs::observable<TileView>  {
public:
    int id;
    int layer;
    Tile::Type type;
    RectI bounds;
public:
    void removeFromScreen();
};

class TankView;
class TankModel : public obs::observable<TankView> {
public:
    int id;
    int hp;
    Tank::Type type;
    Tank::Camp camp;
    Tank::Direction dir;
    Vector2f move;
    Vector2f position;
    Vector2i size;
    RectI bounds;
public:
    void modifyPosition();
    void removeFromScreen();
};

class ButtleModel {
public:
    int id;
};

class PropModel {
public:
    int id;
};

class Widget;
class WorldModel {
public:
    typedef DebugQuadTree<TileModel*> TileTree;
    typedef std::list<TankModel*> TankList;
    typedef std::list<ButtleModel*> BulletList;
    typedef std::list<PropModel*> PropList;
public:
    WorldModel();
public:
    TileTree tiles;
    TankList tanks;
    BulletList bullets;
    PropList props;
    RectI bounds;
    Widget* root;
};

//===================================================================
// 资源路径

namespace res {
    std::string soundName(std::string const& key);
    std::string imageName(std::string const& key);
    std::string fontName(std::string const& key);
    std::string levelName(std::string const& key);
    std::string levelName(int level);
}

#endif //SDL2_UI_DATA_H
