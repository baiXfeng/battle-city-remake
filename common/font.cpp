//
// Created by baifeng on 2021/9/30.
//

#include "font.h"
#include "loadres.h"

mge_begin

TTFont::TTFont():
_font(nullptr),
_color({0, 0, 0, 255}),
_fontSize(0) {}

TTFont::~TTFont() {
    this->close();
}

void TTFont::open(std::string const& fileName, int fontSize) {
    this->close();
    _font = TTF_OpenFont(fileName.c_str(), fontSize);
    _fontSize = fontSize;
}

void TTFont::close() {
    if (_font != nullptr) {
        TTF_CloseFont(_font);
        _font = nullptr;
    }
}

void TTFont::setFontPath(std::string const& path) {
    _fontPath = path;
}

void TTFont::setColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    setColor(SDL_Color{r, g, b, a});
}

void TTFont::setColor(SDL_Color const& c) {
    memcpy(&_color, &c, sizeof(c));
}

SDL_Color& TTFont::getColor() {
    return _color;
}

SDL_Color const& TTFont::getColor() const {
    return _color;
}

int TTFont::fontSize() const {
    return _fontSize;
}

std::string const& TTFont::fontPath() const {
    return _fontPath;
}

void TTFont::setStyle(int style) {
    if (_font == nullptr) {
        return;
    }
    TTF_SetFontStyle(_font, style);
}

int TTFont::getStyle() const {
    if (_font == nullptr) {
        return 0;
    }
    return TTF_GetFontStyle(_font);
}

void TTFont::setOutline(int outline) {
    if (_font == nullptr) {
        return;
    }
    TTF_SetFontOutline(_font, outline);
}

int TTFont::getOutline() const {
    if (_font == nullptr) {
        return 0;
    }
    return TTF_GetFontOutline(_font);
}

void TTFont::setHinting(int hinting) {
    if (_font == nullptr) {
        return;
    }
    TTF_SetFontHinting(_font, hinting);
}

int TTFont::getHinting() const {
    if (_font == nullptr) {
        return 0;
    }
    return TTF_GetFontHinting(_font);
}

TTFont::TexturePtr TTFont::create(SDL_Renderer* renderer, char const* text) {
    if (_font == nullptr) {
        return nullptr;
    }
    auto surface = TTF_RenderText_Solid(_font, text, _color);
    if (surface == nullptr) {
        return nullptr;
    }
    auto texture = SDL_CreateTextureFromSurface(renderer, surface);
    auto ptr = std::make_shared<Texture>(texture);
    SDL_FreeSurface(surface);
    return ptr;
}

TTFont::TexturePtr TTFont::createWithUTF8(SDL_Renderer* renderer, char const* text) {
    if (_font == nullptr) {
        return nullptr;
    }
    auto surface = TTF_RenderUTF8_Solid(_font, text, _color);
    if (surface == nullptr) {
        return nullptr;
    }
    auto texture = SDL_CreateTextureFromSurface(renderer, surface);
    auto ptr = std::make_shared<Texture>(texture);
    SDL_FreeSurface(surface);
    return ptr;
}

TTFont::TexturePtr TTFont::createWithUNICODE(SDL_Renderer* renderer, Uint16 const* text) {
    if (_font == nullptr) {
        return nullptr;
    }
    auto surface = TTF_RenderUNICODE_Solid(_font, text, _color);
    if (surface == nullptr) {
        return nullptr;
    }
    auto texture = SDL_CreateTextureFromSurface(renderer, surface);
    auto ptr = std::make_shared<Texture>(texture);
    SDL_FreeSurface(surface);
    return ptr;
}

mge_end
