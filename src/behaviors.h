//
// Created by baifeng on 2021/10/9.
//

#ifndef BATTLE_CITY_BEHAVIORS_H
#define BATTLE_CITY_BEHAVIORS_H

#include "common/event.h"
#include "common/behavior.h"
#include "common/audio.h"
#include "data.h"

class TankSpawnBehavior : public mge::SequenceBehavior {
public:
    TankSpawnBehavior(WorldModel::TankList* tanks);
private:
    Status tick(float delta) override;
};

class PlayerSpawnBehavior : public mge::Behavior {
public:
    PlayerSpawnBehavior(WorldModel::TankList* tanks);
private:
    Status tick(float delta) override;
private:
    WorldModel::TankList* _tanks;
};

class TankEventBehavior : public mge::Behavior, public mge::Event::Listener, public mge::AudioSystem::Listener {
public:
    TankEventBehavior(WorldModel::TileTree* tiles);
    ~TankEventBehavior();
private:
    Status tick(float delta) override;
    void onEvent(mge::Event const& e) override;
    void onMixFinished(std::string const& name) override;
private:
    bool _openingBgmFinished;
    WorldModel::TileTree* _tiles;
};

class EnemySpawnBehavior : public mge::Behavior {
public:
    EnemySpawnBehavior(WorldModel::TankList* tanks);
    ~EnemySpawnBehavior();
private:
    Status tick(float delta) override;
    int enemyCount() const;
    int enemyRemainCount() const;
    bool is_overlap(mge::RectI const& r) const;
    void checkOverlap(int& index, int& overlapCount) const;
private:
    int _index;
    float _delay, _delay_tick;
    WorldModel::TankList* _tanks;
    AddTankList* _addtanks;
    PlayerModel* _player;
};

class BattleFieldInterface;
class PropCreateBehavior : public mge::Behavior, public mge::Event::Listener {
public:
    PropCreateBehavior(WorldModel* world, BattleFieldInterface* battlefield);
    ~PropCreateBehavior();
private:
    Status tick(float delta) override;
    void onEvent(mge::Event const& e) override;
    void randomPosition(mge::Vector2i& position, int retryCount);
private:
    BattleFieldInterface* _battlefield;
    WorldModel* _world;
};

class TankPowerUpBehavior : public mge::Behavior, public mge::Event::Listener {
public:
    TankPowerUpBehavior(WorldModel* world, BattleFieldInterface* battlefield);
    ~TankPowerUpBehavior();
private:
    Status tick(float delta) override;
    void onEvent(mge::Event const& e) override;
private:
    WorldModel* _world;
    BattleFieldInterface* _battlefield;
};

class BaseReinforceBehavior : public mge::Behavior, public mge::Event::Listener {
public:
    BaseReinforceBehavior(WorldModel* world, BattleFieldInterface* battlefield);
    ~BaseReinforceBehavior();
private:
    Status tick(float delta) override;
    void onEvent(mge::Event const& e) override;
    void onBlink();
    void onFinish();
    void onBlinkImp();
private:
    bool _steel;
    WorldModel* _world;
    BattleFieldInterface* _battlefield;
};

class TankAI_Behavior : public mge::Behavior {
public:
    TankAI_Behavior(TankModel* model);
private:
    Status tick(float delta) override;
    void onAiShoot(float delta);
    void onAiMove(float delta);
private:
    WorldModel* _world;
    TankModel* _model;
    float _shootTicks;
    float _moveTicks;
};

class TankMoveBehavior : public mge::Behavior {
public:
    TankMoveBehavior(TankModel* model, mge::RectI const& bounds);
private:
    Status tick(float delta) override;
private:
    WorldModel* _world;
    TankModel* _model;
    mge::RectI _world_bounds;
};

class TankTileCollisionBehavior : public mge::Behavior {
public:
    TankTileCollisionBehavior(TankModel* model, WorldModel::TileTree* tiles);
private:
    Status tick(float delta) override;
private:
    TankModel* _model;
    WorldModel::TileTree* _tiles;
};

class TankCollisionBehavior : public mge::Behavior {
public:
    TankCollisionBehavior(TankModel* model, WorldModel::TankList* tanks);
private:
    Status tick(float delta) override;
private:
    TankModel* _model;
    WorldModel::TankList* _tanks;
    mge::RectI _world_bounds;
};

class TankFireBehavior : public mge::Behavior {
public:
    TankFireBehavior(TankModel* model, WorldModel::BulletList* bullets);
private:
    Status tick(float delta) override;
private:
    TankModel* _model;
    WorldModel::BulletList* _bullets;
};

class BulletMoveBehavior : public mge::Behavior {
public:
    BulletMoveBehavior(BulletModel* model);
private:
    Status tick(float delta) override;
private:
    BulletModel* _model;
};

class BaseBulletCollisionBehavior : public mge::Behavior {
public:
    BaseBulletCollisionBehavior(BulletModel* model, WorldModel* world);
protected:
    void remove_bullet();
    void bullet_explosion();
    void hit_wall();
    void hit_brick();
    void hit_base();
    void hit_tank();
protected:
    BulletModel* _model;
    WorldModel* _world;
};

class BulletWorldCollisionBehavior : public BaseBulletCollisionBehavior {
public:
    BulletWorldCollisionBehavior(BulletModel* model, WorldModel* world);
private:
    Status tick(float delta) override;
};

class BulletTileCollisionBehavior : public BaseBulletCollisionBehavior {
public:
    BulletTileCollisionBehavior(BulletModel* model, WorldModel* world);
private:
    Status tick(float delta) override;
    mge::RectI getBulletBounds() const;
    mge::RectI getBigBulletBounds() const;
};

class BulletTankCollisionBehavior : public BaseBulletCollisionBehavior {
public:
    BulletTankCollisionBehavior(BulletModel* model, WorldModel* world);
private:
    Status tick(float delta) override;
    void bulletHitTank(TankModel* tank);
};

class BulletBulletCollisionBehavior : public BaseBulletCollisionBehavior {
public:
    BulletBulletCollisionBehavior(BulletModel* model, WorldModel* world);
private:
    Status tick(float delta) override;
};

class PropCollisionBehavior : public mge::Behavior {
public:
    PropCollisionBehavior(PropModel* prop, WorldModel::TankList* tanks, WorldModel::PropList* props);
private:
    Status tick(float delta) override;
    void playEffect(Tank::PowerUp type);
private:
    PropModel* _model;
    WorldModel::TankList* _tanks;
    WorldModel::PropList* _props;
};

#endif //BATTLE_CITY_BEHAVIORS_H
