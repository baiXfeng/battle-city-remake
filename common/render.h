//
// Created by baifeng on 2021/7/12.
//

#ifndef SDL2_UI_RENDER_H
#define SDL2_UI_RENDER_H

#include <SDL.h>
#include "types.h"

class RenderCopy {
public:
    RenderCopy();
    virtual ~RenderCopy() {}
public:
    void setTexture(SDL_Texture* texture);
    void setTexture(SDL_Texture* texture, SDL_Rect const& srcrect);
    void setSize(int w, int h);
    void setSize(Vector2i const& size);
    Vector2i const& size() const;
    void setOpacity(int opacity);
    int opacity() const;
public:
    virtual void draw(SDL_Renderer* renderer, Vector2i const& position = {0, 0});
protected:
    int _opacity;
    SDL_Texture* _texture;
    SDL_Rect _srcrect;
    Vector2i _size;
};

class RenderCopyEx : public RenderCopy {
public:
    RenderCopyEx();
public:
    void setScale(Vector2f const& scale);
    Vector2f const& getScale() const;
    void setAnchor(Vector2f const& anchor);
    Vector2f const& getAnchor() const;
    void setAngle(float angle);
    float getAngle() const;
public:
    void draw(SDL_Renderer* renderer, Vector2i const& position = {0, 0}) override;
protected:
    float _angle;
    Vector2f _scale;
    Vector2f _anchor;
};

class RenderFillRect {
public:
    RenderFillRect();
    virtual ~RenderFillRect() {}
public:
    void setSize(int w, int h);
    Vector2i const& size() const;
    void setColor(SDL_Color const& c);
    SDL_Color const& color() const;
public:
    virtual void draw(SDL_Renderer* renderer, Vector2i const& position = {0, 0});
protected:
    Vector2i _size;
    SDL_Color _color;
};

class RenderDrawRect : public RenderFillRect {
public:
    void draw(SDL_Renderer* renderer, Vector2i const& position = {0, 0}) override;
};

#endif //SDL2_UI_RENDER_H
