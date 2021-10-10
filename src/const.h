//
// Created by baifeng on 2021/10/2.
//

#ifndef SDL2_UI_CONST_H
#define SDL2_UI_CONST_H

namespace EventID {
    enum {
        ENEMY_NUMBER_CHANGED = 0x1001,
        PLAYER1_NUMBER_CHANGED,
        GAME_OVER,
        PAUSE_GAME,
        RESUME_GAME,
        BASE_FALL,      // 基地击破
    };
}

#endif //SDL2_UI_CONST_H
