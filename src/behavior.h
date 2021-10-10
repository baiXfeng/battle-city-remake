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
    SequenceBehavior(Behaviors const& sequence);
protected:
    Status tick(float delta) override;
    void add(Behavior::Ptr const& behavior);
protected:
    Behaviors _sequence;
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

class BulletMoveBehavior : public Behavior {
public:
    BulletMoveBehavior(BulletModel* model, RectI const& bounds);
private:
    Status tick(float delta) override;
private:
    BulletModel* _model;
    RectI _world_bounds;
};

class BulletCollisionBehavior : public Behavior {
public:
    BulletCollisionBehavior(BulletModel* model, WorldModel* world);
private:
    Status tick(float delta) override;
    Status tileCollision(float delta);
    Status worldCollision(float delta);
    void remove_bullet();
    void bullet_explosion();
    void hit_wall();
    void hit_brick();
    void hit_base();
private:
    BulletModel* _model;
    WorldModel* _world;
    std::vector<std::function<Status(float delta)>> _calls;
};

#endif //SDL2_UI_BEHAVIOR_H
