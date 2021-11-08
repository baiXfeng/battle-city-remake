//
// Created by baifeng on 2021/11/8.
//

#include "sound_effect.h"
#include "common/audio.h"
#include "common/game.h"
#include "common/loadres.h"
#include "data.h"

GameSoundEffect::GameSoundEffect() {
    _seNames = {
            "tank-idle",
            "tank-move",
    };
    for (int i = 0; i < 2; ++i) {
        _game.audio().loadMusic( res::bgmName(_seNames[SETYPE(i)]) );
    }
    for (int i = 2; i < _seNames.size(); ++i) {
        _game.audio().loadEffect( res::soundName(_seNames[SETYPE(i)]) );
    }
}

void GameSoundEffect::playSE(SETYPE type) {
    if (type >= _seNames.size()) {
        return;
    }
    if (type == TANK_IDLE_SE or type == TANK_MOVE_SE) {
        _game.audio().playMusic( res::bgmName(_seNames[type]) );
        return;
    }
    _game.audio().playEffect( res::soundName(_seNames[type]) );
}

GameSoundEffect& GameSoundEffect::single() {
    static GameSoundEffect se;
    return se;
}
