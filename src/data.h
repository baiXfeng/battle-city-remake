//
// Created by baifeng on 2021/10/2.
//

#ifndef SDL2_UI_DATA_H
#define SDL2_UI_DATA_H

#include "common/types.h"
#include "common/quadtree.h"
#include "obs.h"
#include "lutok3.h"
#include <vector>
#include <string>

//===================================================================
// 关卡配置

namespace Tile {
#define IC(name, value) static int const name = value
    IC(SIZE, 40);
    IC(MAP_SIZE, SIZE * 13);
    enum Type {
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
    enum Party {
        PLAYER = 0,
        ENEMY
    };
    enum Tier {
        A = 0,
        B,
        C,
        D,
        TIER_MAX,
    };
    enum Direction {
        UP = 0,
        RIGHT,
        DOWN,
        LEFT,
        MAX,
    };
    enum Controller {
        P1 = 0,
        P2,
        AI,
    };
    class Attribute {
    public:
        int health;
        int bulletMaxCount;
        int bulletTankDamage;
        int bulletWallDamage;
        float moveSpeed;
        float bulletRapidFireDelay;
        float bulletSpeed;
    public:
        Attribute();
        Attribute(lutok3::State& table);
    };

    typedef std::vector<Vector2f> Spawns;
    typedef std::vector<Attribute> Attributes;
    typedef std::vector<Attributes> Config;

    void loadTankSpawns();
    Spawns const& getSpawns(Party group);

    void loadAttributes();
    Attribute const& getAttribute(Party group, Tier tier);

    int getDefaultLifeMax();
}

typedef struct {
    Tile::Type type;
    int x;
    int y;
} AddTile;

typedef struct {
    bool has_drop;
    Tank::Tier tier;
} AddTank;

typedef std::vector<AddTile> AddTileList;
typedef std::vector<AddTank> AddTankList;

//===================================================================
// 对象数据

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
    bool fire;
    bool shield;
    bool has_drop;
    Tank::Party party;
    Tank::Tier tier;
    Tank::Direction dir;
    Tank::Controller controller;
    Vector2f move;
    Vector2f position;
    Vector2i size;
    RectI bounds;
public:
    TankModel();
    void modifyPosition();
    void removeFromScreen();
    void createBullet();
};

class TankBuildInfo {
public:
    bool has_drop;
    Tank::Party party;
    Tank::Tier tier;
    Tank::Controller controller;
    Tank::Direction direction;
    Vector2f position;
};

class BulletView;
class BulletModel : public obs::observable<BulletView> {
public:
    int id;
    int sender_id;
    Tank::Party party;
    Vector2f move;
    Vector2f position;
    RectI bounds;
public:
    void modifyPosition();
    void removeFromScreen();
    void playExplosion();
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
    typedef std::list<BulletModel*> BulletList;
    typedef std::list<PropModel*> PropList;
public:
    WorldModel();
public:
    TileTree tiles;
    TankList tanks;
    BulletList bullets;
    PropList props;
    RectI bounds;
};

class PlayerModel {
public:
    int life;
    int killCount[Tank::TIER_MAX];   // 击落数
};

typedef Tank::Spawns PlayerSpawns;
typedef Tank::Spawns EnemySpawns;

//===================================================================
// 资源路径

namespace res {
    std::string soundName(std::string const& key);
    std::string imageName(std::string const& key);
    std::string fontName(std::string const& key);
    std::string levelName(std::string const& key);
    std::string levelName(int level);
    std::string scriptName(std::string const& key);
    std::string assetsName(std::string const& fileName);
}

#endif //SDL2_UI_DATA_H
