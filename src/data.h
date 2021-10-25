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

    typedef std::vector<Vector2f> Spawns;
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

class Widget;
class TileModel: public obs::observable<Widget>  {
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
    void modifyDir(Tank::Direction dir);
    void modifyShield();
    void modifyPosition();
    void removeFromScreen();
    void modifyTier();
    void createBullet();
    void createExplosion();
    void createScore();
    void openShield(float duration);
    void pause();
    void resume();
};

class BulletView;
class BulletModel : public obs::observable<BulletView> {
public:
    int id;
    int sender_id;
    int wall_damage;
    Tank::Party party;
    Vector2f move;
    Vector2f position;
    RectI bounds;
public:
    void modifyPosition();
    void removeFromScreen();
    void playExplosion();
};

class PropView;
class PropModel : public obs::observable<PropView> {
public:
    int id;
    Tank::PowerUp type;
    RectI bounds;
public:
    void createScore();
    void removeFromScreen();
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
    bool sleep;
    TileTree tiles;
    TankList tanks;
    BulletList bullets;
    PropList props;
    TankModel* player[Tank::Controller::P1 + 1];
    TileModel* base;
    RectI bounds;
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

namespace res {
    std::string soundName(std::string const& key);
    std::string imageName(std::string const& key);
    std::string fontName(std::string const& key);
    std::string levelName(std::string const& key);
    std::string levelName(int level);
    std::string scriptName(std::string const& key);
    std::string powerupName(Tank::PowerUp e);
    std::string assetsName(std::string const& fileName);
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
    Vector2f position;
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

#endif //SDL2_UI_DATA_H
