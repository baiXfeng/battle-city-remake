//
// Created by baifeng on 2021/9/24.
//

#include "view.h"
#include "render.h"
#include "game.h"
#include "assert.h"
#include "loadres.h"

static int _widgetCount = 0;

void TestWidget() {
    auto root = std::make_shared<Widget>();
    auto first = std::make_shared<Widget>();
    auto second = std::make_shared<Widget>();
    root->addChild(first);
    first->addChild(second);
    assert(first->children().size() == 1 and "widget children size error.");

    first->addChild(second);
    assert(first->children().size() == 1 and "widget children size error.");

    first->setPosition(100, 100);
    second->setPosition(20, 20);
    first->update(0.0f);
    auto second_position = second->global_position().to<int>();
    Vector2i target_position{120, 120};
    assert(second_position == target_position and "widget globao position error.");

    root->removeChild(first);
    root = nullptr;
    first = nullptr;
    second = nullptr;
    assert(_widgetCount == 0 and "widget memory leak.");
}

//=====================================================================================

Widget::Widget():
        _parent(nullptr),
        _visible(true),
        _update(false),
        _dirty(true),
        _position({0.0f, 0.0f}),
        _global_position({0.0f, 0.0f}),
        _size({0.0f, 0.0f}),
        _anchor({0.0f, 0.0f}),
        _scale({1.0f, 1.0f}) {
    _children.reserve(10);
    _widgetCount++;
#if defined(__vita__)
    this->setSize(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
#else
    int width = 0, height = 0;
    SDL_RenderGetLogicalSize(_game.renderer(), &width, &height);
    this->setSize(width, height);
#endif
}

Widget::~Widget() {
    removeAllChildren();
    _widgetCount--;
}

Widget* Widget::parent() const {
    return _parent;
}

bool Widget::visible() const {
    return _visible;
}

void Widget::enableUpdate(bool update) {
    _update = update;
}

void Widget::setVisible(bool visible) {
    _visible = visible;
}

void Widget::addChild(WidgetPtr& widget) {
    if (widget->_parent) {
        widget->_parent->removeChild(widget);
        widget->_parent = nullptr;
    }
    _children.push_back(widget);
    widget->_parent = this;
}

void Widget::removeChild(WidgetPtr& widget) {
    this->removeChild(widget.get());
}

void Widget::removeChild(Widget* widget) {
    for (auto iter = _children.begin(); iter != _children.end(); iter++) {
        if (iter->get() == widget) {
            widget->_parent = nullptr;
            _children.erase(iter);
            return;
        }
    }
}

void Widget::removeAllChildren() {
    for (auto& child : _children) {
        child->_parent = nullptr;
    }
    _children.clear();
}

void Widget::removeFromParent() {
    if (_parent) {
        _parent->removeChild(this);
    }
}

Widget::WidgetArray& Widget::children() {
    return _children;
}

void Widget::update(float delta) {
    if (_dirty) {
        this->modifyPosition();
        _dirty = false;
    }
    if (not _visible or not _update) {
        return;
    }
    this->onUpdate(delta);
    auto list = _children;
    for (auto& child : list) {
        child->update(delta);
    }
}

void Widget::draw(SDL_Renderer* renderer) {
    if (not _visible) {
        return;
    }
    this->onDraw(renderer);
    for (auto& child : _children) {
        child->draw(renderer);
    }
}

void Widget::onUpdate(float delta) {

}

void Widget::onDraw(SDL_Renderer* renderer) {

}

void Widget::onDirty() {

}

void Widget::modifyPosition() {
    if (not _parent) {
        return;
    }
    _global_position = _parent->_global_position + (_position - _size * _scale.self_abs() * _anchor);
    this->onDirty();
    for (auto& child : _children) {
        child->modifyPosition();
    }
}

void Widget::onModifyPosition(Vector2f const& position) {

}

void Widget::setPosition(Vector2f const& position) {
    _position = position;
    _dirty = true;
    this->onModifyPosition(_position);
}

void Widget::setPosition(float dx, float dy) {
    _position.x = dx;
    _position.y = dy;
    _dirty = true;
    this->onModifyPosition(_position);
}

Vector2f const& Widget::position() const {
    return _position;
}

Vector2f const& Widget::global_position() const {
    return _global_position;
}

void Widget::onModifySize(Vector2f const& size) {

}

void Widget::setSize(Vector2f const& size) {
    _size = size;
    _dirty = true;
    this->onModifySize(_size);
}

void Widget::setSize(float sx, float sy) {
    _size.x = sx;
    _size.y = sy;
    _dirty = true;
    this->onModifySize(_size);
}

Vector2f const& Widget::size() const {
    return _size;
}

void Widget::onModifyAnchor(Vector2f const& anchor) {

}

void Widget::setAnchor(Vector2f const& anchor) {
    _anchor = anchor;
    _dirty = true;
    this->onModifyAnchor(_anchor);
}

void Widget::setAnchor(float x, float y) {
    _anchor.x = x;
    _anchor.y = y;
    _dirty = true;
    this->onModifyAnchor(_anchor);
}

Vector2f const& Widget::anchor() const {
    return _anchor;
}

void Widget::onModifyScale(Vector2f const& scale) {

}

void Widget::setScale(Vector2f const& scale) {
    _scale = scale;
    _dirty = true;
    this->onModifyScale(_scale);
}

void Widget::setScale(float x, float y) {
    _scale.x = x;
    _scale.y = y;
    _dirty = true;
    this->onModifyScale(_scale);
}

Vector2f const& Widget::scale() const {
    return _scale;
}

//=====================================================================================

WindowWidget::WindowWidget() {
    enableUpdate(true);
}

//=====================================================================================

ImageWidget::ImageWidget(TexturePtr const& texture):_target(std::make_shared<RenderCopy>()) {
    this->setTexture(texture);
}

ImageWidget::ImageWidget(TexturePtr const& texture, SDL_Rect const& srcrect):_target(std::make_shared<RenderCopy>()) {
    this->setTexture(texture, srcrect);
}

void ImageWidget::setTexture(TexturePtr const& texture) {
    _target->setTexture(texture->data());
    this->_texture = texture;
    this->setSize(_target->size().to<float>());
}

void ImageWidget::setTexture(TexturePtr const& texture, SDL_Rect const& srcrect) {
    _target->setTexture(texture->data(), srcrect);
    this->_texture = texture;
    this->setSize(_target->size().to<float>());
}

void ImageWidget::onDirty() {
    _target->setSize(_size.x * fabs(_scale.x), _size.y * fabs(_scale.y));
}

void ImageWidget::onDraw(SDL_Renderer* renderer) {
    _target->draw(renderer, global_position().to<int>());
}

//=====================================================================================

ButtonWidget::ButtonWidget(TexturePtr const& normal, TexturePtr const& pressed, TexturePtr const& disabled):_enable(true) {
    bool visibles[3] = {true, false, false};
    for (int i = 0; i < 3; ++i) {
        auto imageView = Ptr(new ImageWidget(nullptr));
        imageView->setVisible(visibles[i]);
        Widget::addChild(imageView);
    }
    setNormalTexture(normal);
    setPressedTexture(pressed);
    setDisabledTexture(disabled);
}

void ButtonWidget::setNormalTexture(TexturePtr const& normal) {
    auto imageView = _children[0]->to<ImageWidget>();
    imageView->setTexture(normal);
    this->setSize(imageView->size());
}

void ButtonWidget::setPressedTexture(TexturePtr const& pressed) {
    auto imageView = _children[1]->to<ImageWidget>();
    imageView->setTexture(pressed);
}

void ButtonWidget::setDisabledTexture(TexturePtr const& disabled) {
    auto imageView = _children[2]->to<ImageWidget>();
    imageView->setTexture(disabled);
}

void ButtonWidget::setEnable(bool enable) {
    _enable = enable;
    setPressed(false);
    _children[0]->setVisible(_enable);
}

void ButtonWidget::setPressed(bool pressed) {
    _children[0]->setVisible(!pressed);
    _children[1]->setVisible(pressed);
    _children[2]->setVisible(!_enable);
}

bool ButtonWidget::enable() const {
    return _enable;
}

bool ButtonWidget::pressed() const {
    return _children[1]->visible();
}

void ButtonWidget::setClick(CallBack const& cb) {
    _callback = cb;
}

void ButtonWidget::click() {
    if (_callback != nullptr) {
        _callback();
    }
}

//=====================================================================================

MaskWidget::MaskWidget(SDL_Color const& c):_color(c) {

}

void MaskWidget::onDraw(SDL_Renderer* renderer) {
    DrawColor dc(renderer);
    auto const size = _size * _scale.self_abs();
    SDL_FRect dst{
        global_position().x,
        global_position().y,
        size.x,
        size.y
    };
    dc.setColor(_color);
    SDL_RenderFillRectF(renderer, &dst);
}