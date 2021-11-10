//
// Created by baifeng on 2021/11/8.
//

#ifndef SDL2_UI_SOUND_EFFECT_H
#define SDL2_UI_SOUND_EFFECT_H

#include <vector>
#include <string>

class SingleSoundEffect {
public:
    typedef std::vector<std::string> SE_Strings;
    enum SETYPE {
        TANK_IDLE_SE = 0,
        TANK_MOVE_SE,
        OPENING_BGM,
        TANK_FIRE_SE,
        BULLET_HIT_WALL_SE, // bullet_hit_1
        BULLET_HIT_BRICK_SE,// bullet_hit_2
        TANK_EXPLOSION,     // explosion_1
        BASE_EXPLOSION,     // explosion_2
        POWERUP_APPEAR,
        POWERUP_PICK,
        LIFEUP_SE,
        SCORE_SE,
        PAUSE_SE,
        GAMEOVER_SE,
        TYPE_MAX,
    };
    void playSE(SETYPE type);
    void stopSE(SETYPE type);
    void stopTankSE();
    static SingleSoundEffect& single();
private:
    SingleSoundEffect();
    int _currType;
    SE_Strings _seNames;
};

#define _SE SingleSoundEffect::single()

#endif //SDL2_UI_SOUND_EFFECT_H
