//
// Created by baifeng on 2021/7/12.
//

#ifndef SDL2_UI_LOADRES_H
#define SDL2_UI_LOADRES_H

#include <SDL.h>
#include <SDL_image.h>
#include "texture.h"
#include "font.h"
#include "types.h"

mge_begin

namespace res {
    std::string const& getAssetsPath();

    Texture::Ptr load_texture(SDL_Renderer* renderer, std::string const& fileName);
    void free_texture(std::string const& name);
    void free_texture(Texture::Ptr const& texture);
    void free_all_texture();

    TTFont::Ptr load_ttf_font(std::string const& fileName, unsigned int size);
    void free_ttf_font(TTFont::Ptr const& font);
    void free_all_ttf_font();
};

mge_end

#endif //SDL2_UI_LOADRES_H
