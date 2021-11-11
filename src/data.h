//
// Created by baifeng on 2021/10/2.
//

#ifndef BATTLE_CITY_DATA_H
#define BATTLE_CITY_DATA_H

#include "common/types.h"
#include "common/quadtree.h"
#include "common/observer.h"
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

namespace Debug {
    class Cheat;
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
        STANDBY,
    };
    enum PowerUp {
        GRENADE = 0,
        HELMET,
        SHOVEL,
        STAR,
        TANK,
        TIMER,
        POWER_MAX,
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

    typedef std::vector<mge::Vector2f> Spawns;
    typedef std::vector<Attribute> Attributes;
    typedef std::vector<Attributes> Config;

    void loadTankSpawns();
    Spawns const& getSpawns(Party group);

    void loadAttributes();
    Attribute const& getAttribute(Party group, Tier tier);

    int getDefaultLifeMax();
    float getPowerUpDuration(std::string const& name);
    float getGlobalFloat(std::string const& name);
    int getGlobalInt(std::string const& name);

    void resetPlayerScore();
    void playerScoreAdd(int score);

    Debug::Cheat& getCheat();
    void resetCheat();
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

namespace mge {
    class Widget;
}

class TileModel: public mge::Observer<mge::Widget>  {
public:
    int id;
    int layer;
    bool visible;       // 显示属性
    Tile::Type type;
    mge::RectI bounds;
public:
    void removeFromScreen();
    void modifyDisplay();
};

class TankView;
class TankModel : public mge::Observer<TankView> {
public:
    int id;
    int hp;
    bool fire;
    bool shield;
    bool has_drop;
    bool visible;
    bool moving;
    Tank::Party party;
    Tank::Tier tier;
    Tank::Direction dir;
    Tank::Controller controller;
    mge::Vector2f move;
    mge::Vector2f position;
    mge::Vector2i size;
    mge::RectI bounds;
public:
    TankModel();
    void modifyDir(Tank::Direction dir);
    void modifyShield();
    void modifyPosition();
    void removeFromScreen();
    void createBullet();
    void createExplosion();
    void createScore();
    void openShield(float duration);
    void onIceFloor();
};

class BulletView;
class BulletModel : public mge::Observer<BulletView> {
public:
    int id;
    int sender_id;
    int wall_damage;
    bool destroy_steel;
    Tank::Party party;
    mge::Vector2f move;
    mge::Vector2f position;
    mge::RectI bounds;
public:
    BulletModel();
    void modifyPosition();
    void removeFromScreen();
    void playExplosion();
};

class PropView;
class PropModel : public mge::Observer<PropView> {
public:
    int id;
    Tank::PowerUp type;
    mge::RectI bounds;
public:
    void createScore();
    void removeFromScreen();
};

class WorldModel {
public:
    typedef mge::DebugQuadTree<TileModel*> TileTree;
    typedef std::list<TankModel*> TankList;
    typedef std::list<BulletModel*> BulletList;
    typedef std::list<PropModel*> PropList;
public:
    WorldModel();
public:
    bool sleep;
    TileTree tiles;
    TankList tanks;
    BulletList bullets;
    PropList props;
    TankModel* player[Tank::Controller::P1 + 1];
    TileModel* base;
    mge::RectI bounds;
};

class PlayerModel {
public:
    bool win;
    int life;
    int killCount[Tank::TIER_MAX];   // 击落数
    Tank::Tier tier;
    PlayerModel();
};

typedef Tank::Spawns PlayerSpawns;
typedef Tank::Spawns EnemySpawns;

//===================================================================
// 资源路径

namespace mge {
    namespace res {
        std::string soundName(std::string const& key);
        std::string bgmName(std::string const& key);
        std::string imageName(std::string const& key);
        std::string fontName(std::string const& key);
        std::string levelName(std::string const& key);
        std::string levelName(int level);
        std::string scriptName(std::string const& key);
        std::string powerupName(Tank::PowerUp e);
        std::string assetsName(std::string const& fileName);
    }
}

//===================================================================
// 事件参数

class TankBuildInfo {
public:
    bool has_drop;
    Tank::Party party;
    Tank::Tier tier;
    Tank::Controller controller;
    Tank::Direction direction;
    mge::Vector2f position;
};

class BulletHitTankInfo {
public:
    BulletModel* bullet;
    TankModel* tank;
    WorldModel* world;
    BulletHitTankInfo(BulletModel* bullet, TankModel* tank, WorldModel* world):
    bullet(bullet), tank(tank), world(world) {}
};

class TankPowerUpInfo {
public:
    TankModel* tank;
    PropModel* prop;
    TankPowerUpInfo(TankModel* tank, PropModel* prop):tank(tank), prop(prop) {}
};

#endif //BATTLE_CITY_DATA_H
