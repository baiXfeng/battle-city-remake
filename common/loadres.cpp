//
// Created by baifeng on 2021/9/30.
//

#include "loadres.h"
#include "game.h"
#include <map>
#include <list>
#include <algorithm>

mge_begin

namespace res {

    static std::map<std::string, Texture::Ptr> _textureCache;

    std::string const& getAssetsPath() {
        static std::string assets_path;
#if defined(__vita__)
        if (assets_path.empty()) {
            assets_path = "app0:";
        }
#elif defined(__NS__)
        if (assets_path.empty()) {
            assets_path = "romfs:/";
        }
#endif
        return assets_path;
    }

    Texture::Ptr load_texture(SDL_Renderer *renderer, std::string const& fileName) {
        Texture::Ptr result = _textureCache[fileName];
        if (result != nullptr) {
            return result;
        }
        auto texture = IMG_LoadTexture(renderer, (getAssetsPath() + fileName).c_str());
        result = std::make_shared<Texture>(texture);
        result->setPath(fileName);
        _textureCache[fileName] = result;
        return result;
    }

    Texture::Ptr load_texture(std::string const& fileName) {
        return load_texture(_game.renderer(), fileName);
    }

    void free_texture(std::string const &name) {
        _textureCache.erase(name);
    }

    void free_texture(Texture::Ptr const &texture) {
        _textureCache.erase(texture->path());
    }

    void free_all_texture() {
        _textureCache.clear();
    }

    static std::list<TTFont::Ptr> _ttfFontCache;

    TTFont::Ptr find_ttf_font(std::string const& fontName, unsigned int size) {
        for (auto& iter : _ttfFontCache) {
            if (iter->fontPath() == fontName and iter->fontSize() == size) {
                return iter;
            }
        }
        return nullptr;
    }

    TTFont::Ptr load_ttf_font(std::string const& fileName, unsigned int size) {
        auto ptr = find_ttf_font(fileName, size);
        if (ptr != nullptr) {
            return ptr;
        }
        ptr = TTFont::New(getAssetsPath() + fileName, size);
        ptr->setFontPath(fileName);
        _ttfFontCache.push_back(ptr);
        return ptr;
    }

    void free_ttf_font(TTFont::Ptr const& font) {
        auto iter = std::find(_ttfFontCache.begin(), _ttfFontCache.end(), font);
        if (iter != _ttfFontCache.end()) {
            _ttfFontCache.erase(iter);
        }
    }

    void free_all_ttf_font() {
        _ttfFontCache.clear();
    }
}

mge_end
