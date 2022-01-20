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
    class ContactFilter;
    class ContactListener;
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
    void onDraw(SDL_Renderer* renderer) override;
    void onButtonDown(int key) override;
    void onButtonUp(int key) override;
    void onJoyAxisMotion(JOYIDX joy_id, int x, int y) override;
private:
    entity::Context _c;
    SystemPtr _sys;
    WorkerPool _worker;
    std::shared_ptr<entity::ContactFilter> _filter;
    std::shared_ptr<entity::ContactListener> _listener;
};

#endif //SDL2_UI_WORLD_H
