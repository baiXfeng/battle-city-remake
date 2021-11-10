//
// Created by baifeng on 2021/9/27.
//

#ifndef SDL2_UI_AUDIO_H
#define SDL2_UI_AUDIO_H

#include <SDL.h>
#include <SDL_mixer.h>
#include <string>
#include <memory>
#include <unordered_map>
#include "observer.h"

class Music {
public:
    typedef std::shared_ptr<Music> Ptr;
public:
    Music();
    ~Music();
public:
    static Ptr New() {
        return Ptr(new Music);
    }
    bool load(std::string const& name);
    int play(int loops);
private:
    void free();
private:
    Mix_Music* _music;
};

class SoundEffect {
public:
    typedef std::shared_ptr<SoundEffect> Ptr;
public:
    static Ptr New() {
        return Ptr(new SoundEffect);
    }
    SoundEffect();
    ~SoundEffect();
public:
    bool load(std::string const& name);
    void play();
    void pause();
    void resume();
    bool paused() const;
    int channel() const;
    void resetChannel();
private:
    void free();
private:
    int _channel;
    Mix_Chunk* _chunk;
};

class AudioSystem {
public:
    typedef std::shared_ptr<Music> MusicPtr;
    typedef std::unordered_map<std::string, MusicPtr> MusicCache;
    typedef std::shared_ptr<SoundEffect> EffectPtr;
    typedef std::unordered_map<std::string, EffectPtr> EffectCache;
public:
    class Listener {
    public:
        virtual ~Listener() {}
    public:
        virtual void onMixFinished(std::string const& name) {}
    };
    typedef std::shared_ptr<Observer<Listener>> ObserverPtr;
    typedef std::unordered_map<std::string, ObserverPtr> ObserverMap;
public:
    AudioSystem();
    ~AudioSystem();
public:
    void loadMusic(std::string const& name);
    void playMusic(std::string const& name, int loops = -1);
    void pauseMusic();
    void resumeMusic();
    void releaseMusic(std::string const& name);
public:
    void loadEffect(std::string const& name);
    void playEffect(std::string const& name);
    void releaseEffect(std::string const& name);
    SoundEffect& se(std::string const& name);
public:
    void addListener(std::string const& name, Listener* p);
    void removeListener(std::string const& name, Listener* p);
private:
    friend void onMixChannelFinished(int channel);
    void onChannelFinished(int channel);
private:
    MusicCache _musicCache;
    EffectCache _effectCache;
    ObserverMap _observers;
};

#endif //SDL2_UI_AUDIO_H
