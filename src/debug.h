//
// Created by baifeng on 2021/10/30.
//

#ifndef SDL2_UI_DEBUG_H
#define SDL2_UI_DEBUG_H

#include "data.h"

namespace Debug {
    static bool always_appear_powerup = false; // 总是生成奖励
    static auto appear_powerup_type = Tank::POWER_MAX;    // 出现的奖励类型
    static bool enemy_unmatched = false;       // 敌人无敌
    static bool enemy_misfire = false;         // 敌人不攻击
    static bool player_unmatched = false;      // 玩家无敌
    static bool player_level_max = false;      // 玩家最大等级
    static bool base_unmatched = false;        // 基地无敌
}

#endif //SDL2_UI_DEBUG_H
