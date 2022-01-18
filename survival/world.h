//
// Created by baifeng on 2021/11/13.
//

#ifndef SDL2_UI_WORLD_H
#define SDL2_UI_WORLD_H

#include "common/widget.h"
#include "context.h"

namespace entity {
    class System;
    class Worker;
}
class SurvivalView : public mge::GamePadWidget {
public:
    typedef std::shared_ptr<entity::System> SystemPtr;
    typedef std::shared_ptr<entity::Worker> WorkerPtr;
    typedef std::vector<WorkerPtr> WorkerPool;
public:
    SurvivalView();
private:
    void onUpdate(float delta) override;
    void onButtonDown(int key) override;
    void onButtonUp(int key) override;
    void onJoyAxisMotion(JOYIDX joy_id, int x, int y) override;
private:
    entity::Context _c;
    SystemPtr _sys;
    WorkerPool _worker;
};

#endif //SDL2_UI_WORLD_H
