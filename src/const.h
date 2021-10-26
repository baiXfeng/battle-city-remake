//
// Created by baifeng on 2021/10/2.
//

#ifndef BATTLE_CITY_CONST_H
#define BATTLE_CITY_CONST_H

namespace EventID {
    enum {
        ENEMY_NUMBER_CHANGED = 0x1001,
        PLAYER_LIFE_CHANGED,
        GAME_OVER,
        GAME_OVER_ANIMATION,
        PLAYER_WIN,
        PLAYER_DEAD,
        PAUSE_GAME,
        RESUME_GAME,
        BASE_FALL,      // 基地击破
        TANK_FIRE,
        TANK_GEN,       // 生成坦克
        TANK_POWERUP,   // 升级坦克
        ENEMY_KILLED,   // 杀死敌军坦克
        PROP_GEN,       // 生成奖励
    };
}

#endif //BATTLE_CITY_CONST_H
