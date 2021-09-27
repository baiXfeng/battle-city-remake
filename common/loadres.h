//
// Created by baifeng on 2021/7/12.
//

#ifndef SDL2_UI_LOADRES_H
#define SDL2_UI_LOADRES_H

#include <SDL.h>
#include <SDL_image.h>
#include <memory>
#include <string>
#include <map>

class Texture {
public:
    typedef std::shared_ptr<Texture> Ptr;
public:
    Texture(SDL_Texture* texture):_texture(texture) {}
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

namespace res {
    static std::map<std::string, Texture::Ptr> _textureCache;
    static std::string const& getAssetsPath() {
        static std::string assets_path;
#if defined(__vita__)
        if (assets_path.empty()) {
            assets_path = "app0:";
        }
#endif
        return assets_path;
    }
    static Texture::Ptr load_texture(SDL_Renderer *renderer, std::string const& fileName) {
        Texture::Ptr result = _textureCache[fileName];
        if (result != nullptr) {
            return result;
        }
        auto texture = IMG_LoadTexture(renderer, (getAssetsPath()+fileName).c_str());
        result = std::make_shared<Texture>(texture);
        result->setPath(fileName);
        _textureCache[fileName] = result;
        return result;
    }
    static void free_texture(std::string const& name) {
        _textureCache.erase(name);
    }
    static void free_texture(Texture::Ptr const& texture) {
        _textureCache.erase(texture->path());
    }
};

#endif //SDL2_UI_LOADRES_H
