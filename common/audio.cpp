//
// Created by baifeng on 2021/9/27.
//

#include "audio.h"
#include "loadres.h"
#include "game.h"
#include "action.h"
#include <iostream>

mge_begin

Music::Music():_music(nullptr) {

}

Music::~Music() {
    this->free();
}

bool Music::load(std::string const& name) {
    this->free();
    auto fullname = res::getAssetsPath() + name;
    _music = Mix_LoadMUS(fullname.c_str());
    return _music != nullptr;
}

int Music::play(int loops) {
    return Mix_PlayMusic(_music, loops);
}

void Music::free() {
    if (_music) {
        Mix_FreeMusic(_music);
        _music = nullptr;
    }
}

SoundEffect::SoundEffect():_chunk(nullptr), _channel(-0xA) {

}

SoundEffect::~SoundEffect() {
    this->free();
}

bool SoundEffect::load(std::string const& name) {
    this->free();
    auto fullname = res::getAssetsPath() + name;
    _chunk = Mix_LoadWAV(fullname.c_str());
    return _chunk != nullptr;
}

void SoundEffect::play() {
    _channel = Mix_PlayChannel(-1, _chunk, 0);
}

void SoundEffect::pause() {
    Mix_Pause(_channel);
}

void SoundEffect::resume() {
    Mix_Resume(_channel);
}

bool SoundEffect::paused() const {
    return Mix_Paused(_channel) == 1;
}

int SoundEffect::channel() const {
    return _channel;
}

void SoundEffect::resetChannel() {
    _channel = -0xA;
}

void SoundEffect::free() {
    if (_chunk) {
        Mix_FreeChunk(_chunk);
        _chunk = nullptr;
    }
}

class ChannelCallBack : public Action {
public:
    typedef std::function<void(int)> Callback;
    ChannelCallBack(Callback const& cb, int channel):_channel(channel), _call(cb) {}
private:
    State Step(float dt) override {
        if (_call != nullptr) {
            _call(_channel);
        }
        return FINISH;
    }
private:
    int _channel;
    Callback _call;
};

void onMixChannelFinished(int channel) {
    _game.runActionOnMainThread(Action::Ptr(
            new ChannelCallBack(
                    std::bind(&AudioSystem::onChannelFinished, &_game.audio(), std::placeholders::_1),
                    channel
            )
    ));
}

AudioSystem::AudioSystem() {
    Mix_ChannelFinished(&onMixChannelFinished);
}

AudioSystem::~AudioSystem() {
    Mix_ChannelFinished(nullptr);
}

void AudioSystem::loadMusic(std::string const& name) {
    auto iter = _musicCache.find(name);
    if (iter == _musicCache.end()) {
        _musicCache.insert(std::make_pair(name, Music::New()));
        iter = _musicCache.find(name);
        if (!iter->second->load(name)) {
            std::cout << "Failed to load music: " << name << " [" << SDL_GetError() << "]" << std::endl;
        }
    }
}

void AudioSystem::playMusic(std::string const& name, int loops) {
    auto iter = _musicCache.find(name);
    if (iter == _musicCache.end()) {
        this->loadMusic(name);
        iter = _musicCache.find(name);
    }
    iter->second->play(loops);
}

void AudioSystem::pauseMusic() {
    Mix_PauseMusic();
}

void AudioSystem::resumeMusic() {
    Mix_ResumeMusic();
}

void AudioSystem::releaseMusic(std::string const& name) {
    _musicCache.erase(name);
}

void AudioSystem::loadEffect(std::string const& name) {
    auto iter = _effectCache.find(name);
    if (iter == _effectCache.end()) {
        _effectCache.insert(std::make_pair(name, SoundEffect::New()));
        iter = _effectCache.find(name);
        if (!iter->second->load(name)) {
            std::cout << "Failed to load audio: " << name << " [" << SDL_GetError() << "]" << std::endl;
        }
    }
}

void AudioSystem::playEffect(std::string const& name) {
    auto iter = _effectCache.find(name);
    if (iter == _effectCache.end()) {
        loadEffect(name);
        iter = _effectCache.find(name);
    }
    iter->second->play();

    // reset channel while overlap
    int const channel = iter->second->channel();
    for (auto& se : _effectCache) {
        if (se.first == name) {
            continue;
        }
        if (se.second->channel() == channel) {
            se.second->resetChannel();
        }
    }
}

void AudioSystem::releaseEffect(std::string const& name) {
    _effectCache.erase(name);
}

SoundEffect& AudioSystem::se(std::string const& name) {
    this->loadEffect(name);
    return *_effectCache[name].get();
}

void AudioSystem::addListener(std::string const& name, Listener* p) {
    auto iter = _observers.find(name);
    if (iter == _observers.end()) {
        _observers.insert(std::make_pair(name, std::make_shared<Observer<Listener>>()));
        iter = _observers.find(name);
    }
    iter->second->add(p);
}

void AudioSystem::removeListener(std::string const& name, Listener* p) {
    auto obs = _observers.find(name);
    if (obs != _observers.end()) {
        obs->second->remove(p);
    }
}

void AudioSystem::onChannelFinished(int channel) {
    auto& cache = _effectCache;
    for (auto iter = cache.begin(); iter != cache.end(); iter++) {
        if (iter->second->channel() == channel) {
            int temp_channel = channel;
            auto key = iter->first;
            auto& observers = _observers;
            auto obs = observers.find(iter->first);
            if (obs != observers.end()) {
                obs->second->each([&iter](AudioSystem::Listener* view) {
                    view->onMixFinished(iter->first);
                });
            }
            return;
        }
    }
}

mge_end
