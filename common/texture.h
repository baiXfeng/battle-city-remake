//
// Created by baifeng on 2021/9/30.
//

#ifndef SDL2_UI_TEXTURE_H
#define SDL2_UI_TEXTURE_H

#include <SDL.h>
#include <string>
#include <memory>

class Texture {
public:
    typedef std::shared_ptr<Texture> Ptr;
public:
    Texture(SDL_Texture* texture):_texture(texture) {
#if !defined(__vita__)
        //SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
#endif
    }
    virtual ~Texture() {
        if (_texture) {
            SDL_DestroyTexture(_texture);
            _texture = nullptr;
        }
    }
public:
    SDL_Texture* data() const {
        return _texture;
    }
    void setPath(std::string const& path) {
        _path = path;
    }
    std::string const& path() const {
        return _path;
    }
protected:
    SDL_Texture* _texture;
    std::string _path;
};

#endif //SDL2_UI_TEXTURE_H
