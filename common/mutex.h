//
// Created by baifeng on 2021/11/9.
//

#ifndef SDL2_UI_MUTEX_H
#define SDL2_UI_MUTEX_H

#include <SDL.h>

class Mutex {
    friend class Lock;
public:
    Mutex(): _data(SDL_CreateMutex()) {}
    virtual ~Mutex() {
        SDL_DestroyMutex(_data);
        _data = nullptr;
    }
protected:
    SDL_mutex* _data;
};

class Lock {
public:
    Lock(Mutex& mutex):_mutex(&mutex) {
        SDL_LockMutex(_mutex->_data);
    }
    virtual ~Lock() {
        SDL_UnlockMutex(_mutex->_data);
    }
protected:
    Mutex* _mutex;
};

#endif //SDL2_UI_MUTEX_H
