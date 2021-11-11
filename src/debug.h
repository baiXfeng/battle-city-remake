//
// Created by baifeng on 2021/10/30.
//

#ifndef SDL2_UI_DEBUG_H
#define SDL2_UI_DEBUG_H

#include "data.h"

namespace Debug {
    class Cheat {
    public:
        bool always_appear_powerup = false; // 总是生成奖励
        Tank::PowerUp appear_powerup_type = Tank::POWER_MAX;       // 出现的奖励类型
        bool enemy_unmatched = false;       // 敌人无敌
        bool enemy_misfire = false;         // 敌人不攻击
        bool player_unmatched = false;      // 玩家无敌
        bool player_level_max = false;      // 玩家最大等级
        bool base_unmatched = false;        // 基地无敌
    };
}

#endif //SDL2_UI_DEBUG_H
