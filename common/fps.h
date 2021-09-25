//
// Created by baifeng on 2021/5/12.
//

#ifndef MGS_FPS_H
#define MGS_FPS_H

#include <SDL.h>

class Fps {
public:
    explicit Fps(int limit = 60):
            _frameDelay(1000.0f/limit),
            _frameStart(0),
            _frameTime(0),
            _delta(_frameDelay / 1000.0f) {}
    void start() {
        _frameStart = SDL_GetTicks();
    }
    void next() {
        int const _delay = _frameDelay;
        _frameTime = SDL_GetTicks() - _frameStart;
        if (_delay > _frameTime) {
            SDL_Delay(_delay - _frameTime);
        }
    }
    float delta() const {
        return _frameDelay > _frameTime ? _delta : _frameTime / 1000.0f;
    }
private:
    int _frameStart, _frameTime;
    float _frameDelay, _delta;
};

#endif //MGS_FPS_H
