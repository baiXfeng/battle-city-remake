//
// Created by baifeng on 2021/11/8.
//

#include "sound_effect.h"
#include "common/audio.h"
#include "common/game.h"
#include "data.h"

SingleSoundEffect::SingleSoundEffect(): _currType(TYPE_MAX) {
    _seNames = {
            "tank-idle",
            "tank-move",
            "stage_start",
            "bullet_shot",
            "bullet_hit_1",
            "bullet_hit_2",
            "explosion_1",
            "explosion_2",
            "powerup_appear",
            "powerup_pick",
            "life",
            "score",
            "pause",
            "game_over",
    };
    for (int i = 0; i < 2; ++i) {
        _game.audio().loadMusic( res::bgmName(_seNames[SETYPE(i)]) );
    }
    for (int i = 2; i < _seNames.size(); ++i) {
        _game.audio().loadEffect( res::soundName(_seNames[SETYPE(i)]) );
    }
}

void SingleSoundEffect::playSE(SETYPE type) {
    if (type >= _seNames.size()) {
        return;
    }
    if (type == TANK_IDLE_SE or type == TANK_MOVE_SE) {
        if (_currType == type) {
            return;
        }
        _currType = type;
        _game.audio().playMusic( res::bgmName(_seNames[type]) );
        return;
    }
    _game.audio().playEffect( res::soundName(_seNames[type]) );
}

void SingleSoundEffect::stopSE(SETYPE type) {
    if (type >= _seNames.size()) {
        return;
    }
    if (type == TANK_IDLE_SE or type == TANK_MOVE_SE) {
        return;
    }
    _game.audio().releaseEffect( res::soundName(_seNames[type]) );
}

void SingleSoundEffect::stopTankSE() {
    _game.audio().pauseMusic();
}

SingleSoundEffect& SingleSoundEffect::single() {
    static SingleSoundEffect se;
    return se;
}
