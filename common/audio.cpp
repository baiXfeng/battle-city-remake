//
// Created by baifeng on 2021/9/27.
//

#include "audio.h"
#include "loadres.h"
#include <iostream>

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

SoundEffect::SoundEffect():_chunk(nullptr), _channel(0) {

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

void SoundEffect::free() {
    if (_chunk) {
        Mix_FreeChunk(_chunk);
        _chunk = nullptr;
    }
}


AudioSystem::AudioSystem() {
    int result = Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 1024 );
    // Check load
    if ( result != 0 ) {
        std::cout << "Failed to open audio: " << Mix_GetError() << std::endl;
    }
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
}

void AudioSystem::releaseEffect(std::string const& name) {
    _effectCache.erase(name);
}