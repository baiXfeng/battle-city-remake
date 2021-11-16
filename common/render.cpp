//
// Created by baifeng on 2021/7/12.
//

#include "render.h"
#include "texture.h"

mge_begin

RenderCopy::RenderCopy():
_texture(nullptr),
_srcrect({0,0,0,0}),
_size({0,0}),
_opacity(255) {}

Texture::Ptr& RenderCopy::texture() {
    return _texture;
}

Texture::Ptr const& RenderCopy::texture() const {
    return _texture;
}

void RenderCopy::setTexture(TexturePtr const& texture) {
    _texture = texture;
    if (_texture != nullptr and _texture->data()) {
        int width = 0;
        int height = 0;
        SDL_QueryTexture(_texture->data(), nullptr, nullptr, &width, &height);
        _srcrect.w = width;
        _srcrect.h = height;
        _size = {width, height};
    }
}

void RenderCopy::setTexture(TexturePtr const& texture, SDL_Rect const& srcrect) {
    _texture = texture;
    _srcrect = srcrect;
    _size = {srcrect.w, srcrect.h};
}

void RenderCopy::setSize(int w, int h) {
    _size.x = w;
    _size.y = h;
}

void RenderCopy::setSize(Vector2i const& size) {
    _size = size;
}

Vector2i const& RenderCopy::size() const {
    return _size;
}

void RenderCopy::setOpacity(int opacity) {
    _opacity = opacity % 256;
}

int RenderCopy::opacity() const {
    return _opacity;
}

void RenderCopy::draw(SDL_Renderer* renderer, Vector2i const& position) {
    if (_texture == nullptr) {
        return;
    }
    SDL_Rect dstrect{
        position.x,
        position.y,
        _size.x,
        _size.y
    };
    SDL_SetTextureAlphaMod(_texture->data(), _opacity);
    SDL_RenderCopy(renderer, _texture->data(), &_srcrect, &dstrect);
}

RenderCopyEx::RenderCopyEx():
_angle(0.0f),
_scale({1.0f,1.0f}),
_anchor({0.0f,0.0f}) {}

void RenderCopyEx::setScale(Vector2f const& scale) {
    _scale = scale;
}

Vector2f const& RenderCopyEx::getScale() const {
    return _scale;
}

void RenderCopyEx::setAnchor(Vector2f const& anchor) {
    _anchor = anchor;
}

Vector2f const& RenderCopyEx::getAnchor() const {
    return _anchor;
}

void RenderCopyEx::setAngle(float angle) {
    _angle = angle;
}

float RenderCopyEx::getAngle() const {
    return _angle;
}

void RenderCopyEx::draw(SDL_Renderer* renderer, Vector2i const& position) {
    if (_texture == nullptr) {
        return;
    }
    int width = _size.x * abs(_scale.x);
    int height = _size.y * abs(_scale.y);
    SDL_Point center{
            int(width * _anchor.x),
            int(height * _anchor.y)
    };
    SDL_Rect dstrect{
            position.x - center.x,
            position.y - center.y,
            width,
            height
    };
    auto flip = SDL_RendererFlip::SDL_FLIP_NONE;
    if (_scale.x < 0) {
        flip = SDL_RendererFlip::SDL_FLIP_HORIZONTAL;
    } else if (_scale.y < 0) {
        flip = SDL_RendererFlip::SDL_FLIP_VERTICAL;
    }
    SDL_SetTextureAlphaMod(_texture->data(), _opacity);
    SDL_RenderCopyEx(renderer, _texture->data(), &_srcrect, &dstrect, _angle, &center, flip);
}

RenderFillRect::RenderFillRect(): _color({0, 0, 0, 255}) {

}

void RenderFillRect::setSize(int w, int h) {
    _size = {w, h};
}

Vector2i const& RenderFillRect::size() const {
    return _size;
}

void RenderFillRect::setColor(SDL_Color const& c) {
    _color = c;
}

SDL_Color const& RenderFillRect::color() const {
    return _color;
}

void RenderFillRect::draw(SDL_Renderer* renderer, Vector2i const& position) {
    DrawColor dc(renderer);
    dc.setColor(_color);
    SDL_Rect rect{
        position.x,
        position.y,
        _size.x,
        _size.y,
    };
    SDL_RenderFillRect(renderer, &rect);
}

void RenderDrawRect::draw(SDL_Renderer* renderer, Vector2i const& position) {
    DrawColor dc(renderer);
    dc.setColor(_color);
    SDL_Rect rect{
            position.x,
            position.y,
            _size.x,
            _size.y,
    };
    SDL_RenderDrawRect(renderer, &rect);
}

mge_end
