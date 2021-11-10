//
// Created by baifeng on 2021/9/30.
//

#ifndef SDL2_UI_FONT_H
#define SDL2_UI_FONT_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <memory>
#include <string>
#include "types.h"

mge_begin

class Texture;
class TTFont {
public:
    typedef std::shared_ptr<TTFont> Ptr;
    typedef std::shared_ptr<Texture> TexturePtr;
public:
    TTFont();
    virtual ~TTFont();
public:
    static Ptr New(std::string const& fileName, int fontSize) {
        Ptr font(new TTFont);
        font->open(fileName, fontSize);
        return font;
    }
    void open(std::string const& fileName, int fontSize);
    void close();
public:
    void setColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    void setColor(SDL_Color const& c);
    SDL_Color& getColor();
    SDL_Color const& getColor() const;
    int fontSize() const;
    std::string const& fontPath() const;
public:
    void setStyle(int style);
    int getStyle() const;
    void setOutline(int outline);
    int getOutline() const;
    void setHinting(int hinting);
    int getHinting() const;
public:
    TexturePtr create(SDL_Renderer* renderer, char const* text);
    TexturePtr createWithUTF8(SDL_Renderer* renderer, char const* text);
    TexturePtr createWithUNICODE(SDL_Renderer* renderer, Uint16 const* text);
protected:
    int _fontSize;
    TTF_Font* _font;
    SDL_Color _color;
    std::string _fontPath;
};

mge_end

#endif //SDL2_UI_FONT_H
