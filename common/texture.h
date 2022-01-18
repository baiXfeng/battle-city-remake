//
// Created by baifeng on 2021/9/30.
//

#ifndef SDL2_UI_TEXTURE_H
#define SDL2_UI_TEXTURE_H

#include <SDL.h>
#include <string>
#include <memory>
#include "macro.h"
#include "vector2.h"

mge_begin

class Texture {
public:
    typedef std::shared_ptr<Texture> Ptr;
public:
    Texture(SDL_Texture* texture):_texture(texture) {
        if (_texture) {
            SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
            SDL_QueryTexture(_texture, nullptr, nullptr, &_size.x, &_size.y);
        }
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
    Vector2i const& size() const {
        return _size;
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
    Vector2i _size;
};

mge_end

#endif //SDL2_UI_TEXTURE_H
