//
// Created by baifeng on 2021/10/9.
//

#ifndef BATTLE_CITY_BEHAVIORS_H
#define BATTLE_CITY_BEHAVIORS_H

#include "common/event.h"
#include "common/behavior.h"
#include "data.h"

class TankSpawnBehavior : public SequenceBehavior {
public:
    TankSpawnBehavior(WorldModel::TankList* tanks);
private:
    Status tick(float delta) override;
};

class PlayerSpawnBehavior : public Behavior {
public:
    PlayerSpawnBehavior(WorldModel::TankList* tanks);
private:
    Status tick(float delta) override;
private:
    WorldModel::TankList* _tanks;
};

class EnemySpawnBehavior : public Behavior {
public:
    EnemySpawnBehavior(WorldModel::TankList* tanks);
    ~EnemySpawnBehavior();
private:
    Status tick(float delta) override;
    int enemyCount() const;
    int enemyRemainCount() const;
    bool is_overlap(RectI const& r) const;
    void checkOverlap(int& index, int& overlapCount) const;
private:
    int _index;
    float _delay, _delay_tick;
    WorldModel::TankList* _tanks;
    AddTankList* _addtanks;
    PlayerModel* _player;
};

class BattleFieldInterface;
class PropCreateBehavior : public Behavior, public Event::Listener {
public:
    PropCreateBehavior(WorldModel* world, BattleFieldInterface* battlefield);
    ~PropCreateBehavior();
private:
    Status tick(float delta) override;
    void onEvent(Event const& e) override;
private:
    BattleFieldInterface* _battlefield;
    WorldModel* _world;
};

class TankPowerUpBehavior : public Behavior, public Event::Listener {
public:
    TankPowerUpBehavior(WorldModel* world, BattleFieldInterface* battlefield);
    ~TankPowerUpBehavior();
private:
    Status tick(float delta) override;
    void onEvent(Event const& e) override;
private:
    WorldModel* _world;
    BattleFieldInterface* _battlefield;
};

class BaseReinforceBehavior : public Behavior, public Event::Listener {
public:
    BaseReinforceBehavior(WorldModel* world, BattleFieldInterface* battlefield);
    ~BaseReinforceBehavior();
private:
    Status tick(float delta) override;
    void onEvent(Event const& e) override;
    void onBlink();
    void onFinish();
    void onBlinkImp();
private:
    bool _steel;
    WorldModel* _world;
    BattleFieldInterface* _battlefield;
};

class TankAI_Behavior : public Behavior {
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

class TankMoveBehavior : public Behavior {
public:
    TankMoveBehavior(TankModel* model, RectI const& bounds);
private:
    Status tick(float delta) override;
private:
    WorldModel* _world;
    TankModel* _model;
    RectI _world_bounds;
};

class TankTileCollisionBehavior : public Behavior {
public:
    TankTileCollisionBehavior(TankModel* model, WorldModel::TileTree* tiles);
private:
    Status tick(float delta) override;
private:
    TankModel* _model;
    WorldModel::TileTree* _tiles;
};

class TankCollisionBehavior : public Behavior {
public:
    TankCollisionBehavior(TankModel* model, WorldModel::TankList* tanks);
private:
    Status tick(float delta) override;
private:
    TankModel* _model;
    WorldModel::TankList* _tanks;
    RectI _world_bounds;
};

class TankFireBehavior : public Behavior {
public:
    TankFireBehavior(TankModel* model, WorldModel::BulletList* bullets);
private:
    Status tick(float delta) override;
private:
    TankModel* _model;
    WorldModel::BulletList* _bullets;
};

class BulletMoveBehavior : public Behavior {
public:
    BulletMoveBehavior(BulletModel* model);
private:
    Status tick(float delta) override;
private:
    BulletModel* _model;
};

class BaseBulletCollisionBehavior : public Behavior {
public:
    BaseBulletCollisionBehavior(BulletModel* model, WorldModel* world);
protected:
    void remove_bullet();
    void bullet_explosion();
    void hit_wall();
    void hit_brick();
    void hit_base();
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
    RectI getBulletBounds() const;
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

class PropCollisionBehavior : public Behavior {
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
