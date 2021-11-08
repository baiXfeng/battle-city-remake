//
// Created by baifeng on 2021/11/8.
//

#ifndef SDL2_UI_SOUND_EFFECT_H
#define SDL2_UI_SOUND_EFFECT_H

#include <vector>
#include <string>

class GameSoundEffect {
public:
    typedef std::vector<std::string> SE_Strings;
    enum SETYPE {
        TANK_IDLE_SE = 0,
        TANK_MOVE_SE,
        TANK_FIRE_SE,
        BULLET_HIT_TANK_SE,
        BULLET_HIT_WALL_SE,
        BULLET_HIT_STEEL_SE,
        TYPE_MAX,
    };
    void playSE(SETYPE type);
    static GameSoundEffect& single();
private:
    GameSoundEffect();
    SE_Strings _seNames;
};

#define _SE GameSoundEffect::single()

#endif //SDL2_UI_SOUND_EFFECT_H
