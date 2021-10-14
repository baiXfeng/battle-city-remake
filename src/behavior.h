//
// Created by baifeng on 2021/10/9.
//

#ifndef SDL2_UI_BEHAVIOR_H
#define SDL2_UI_BEHAVIOR_H

#include <memory>
#include <vector>
#include "data.h"

class Behavior {
public:
    enum Status {
        fail,
        success,
        running
    };
    typedef std::shared_ptr<Behavior> Ptr;
    template<typename T, typename... Args>
    static Ptr New(Args const&... args) {
        return Ptr(new T(args...));
    }
public:
    virtual ~Behavior() {}
    virtual Status tick(float delta) = 0;
};

class SequenceBehavior : public Behavior {
public:
    typedef std::vector<Ptr> Behaviors;
public:
    SequenceBehavior();
    SequenceBehavior(Behaviors const& sequence);
protected:
    Status tick(float delta) override;
    void add(Behavior::Ptr const& behavior);
protected:
    Behaviors _sequence;
};

class TankSpawnBehavior : public SequenceBehavior {
public:
    TankSpawnBehavior(WorldModel::TankList* tanks);
protected:
    Status tick(float delta) override;
};

class EnemySpawnBehavior : public Behavior {
public:
    EnemySpawnBehavior(WorldModel::TankList* tanks);
protected:
    Status tick(float delta) override;
protected:
    int _index;
    WorldModel::TankList* _tanks;
};

class PlayerSpawnBehavior : public Behavior {
public:
    PlayerSpawnBehavior(WorldModel::TankList* tanks);
protected:
    Status tick(float delta) override;
protected:
    WorldModel::TankList* _tanks;
};

class TankAI_None : public Behavior {
    Status tick(float delta) override {
        return success;
    }
};

class TankAI_Behavior : public Behavior {
public:
    TankAI_Behavior(TankModel* model);
private:
    Status tick(float delta) override;
private:
    TankModel* _model;
};

class TankMoveBehavior : public Behavior {
public:
    TankMoveBehavior(TankModel* model, RectI const& bounds);
private:
    Status tick(float delta) override;
private:
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
};

class BulletTankCollisionBehavior : public BaseBulletCollisionBehavior {
public:
    BulletTankCollisionBehavior(BulletModel* model, WorldModel* world);
private:
    Status tick(float delta) override;
};

#endif //SDL2_UI_BEHAVIOR_H
