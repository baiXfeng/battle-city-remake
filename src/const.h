//
// Created by baifeng on 2021/10/2.
//

#ifndef SDL2_UI_CONST_H
#define SDL2_UI_CONST_H

namespace EventID {
    enum {
        ENEMY_NUMBER_CHANGED = 0x1001,
        PLAYER_LIFE_CHANGED,
        GAME_OVER,
        PLAYER_WIN,
        PAUSE_GAME,
        RESUME_GAME,
        BASE_FALL,      // 基地击破
        TANK_FIRE,
        TANK_GEN,       // 生成坦克
        BULLET_HIT_TANK,
    };
}

#endif //SDL2_UI_CONST_H
