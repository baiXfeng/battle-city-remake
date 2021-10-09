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

#endif //SDL2_UI_BEHAVIOR_H
