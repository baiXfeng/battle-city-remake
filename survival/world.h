//
// Created by baifeng on 2021/11/13.
//

#ifndef SDL2_UI_WORLD_H
#define SDL2_UI_WORLD_H

#include "common/widget.h"
#include "context.h"

namespace entity {
    class System;
}
class SurvivalView : public mge::GamePadWidget {
public:
    typedef std::shared_ptr<entity::System> SystemPtr;
public:
    SurvivalView();
private:
    void onUpdate(float delta) override;
    void onButtonDown(int key) override;
    void onButtonUp(int key) override;
private:
    entity::Context _c;
    SystemPtr _sys;
};

#endif //SDL2_UI_WORLD_H
