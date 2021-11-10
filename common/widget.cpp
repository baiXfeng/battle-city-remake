//
// Created by baifeng on 2021/9/24.
//

#include "widget.h"
#include "render.h"
#include "game.h"
#include "assert.h"
#include "loadres.h"
#include "action.h"
#include "font.h"

mge_begin

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
    assert(second_position == target_position and "widget global position error.");

    root->removeChild(first);
    root = nullptr;
    first = nullptr;
    second = nullptr;
    assert(_widgetCount == 0 and "widget memory leak.");
}

//=====================================================================================

Widget::Widget():
_parent(nullptr),
_userdata(nullptr),
_visible(true),
_update(false),
_pause_action_when_hidden(false),
_dirty(true),
_action(ActionExecuterPtr(new ActionExecuter)),
_rotation(0.0f),
_position({0.0f, 0.0f}),
_global_position({0.0f, 0.0f}),
_size({0.0f, 0.0f}),
_global_size({0.0f, 0.0f}),
_anchor({0.0f, 0.0f}),
_scale({1.0f, 1.0f}),
_opacity(255) {
    _children.reserve(10);
    ++_widgetCount;
#if defined(__vita__)
    this->setSize(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
#else
    int width = 0, height = 0;
    SDL_RenderGetLogicalSize(_game.renderer(), &width, &height);
    this->setSize(width, height);
#endif
    _global_size = _size;
}

Widget::~Widget() {
    removeAllChildren();
    --_widgetCount;
    //printf("widget size = %d\n", _widgetCount);
}

Widget* Widget::parent() const {
    return _parent;
}

Widget* Widget::root() {
    Widget* _root = this;
    while (_root->_parent != nullptr) {
        _root = _root->_parent;
    }
    return _root;
}

Widget::Ptr Widget::ptr() const {
    if (_parent != nullptr) {
        for (auto& child : _parent->_children) {
            if (child.get() == this) {
                return child;
            }
        }
    }
    return nullptr;
}

bool Widget::visible() const {
    return _visible;
}

void Widget::defer(std::function<void()> const& func, float delay) {
    auto delay_action = Action::Ptr(new Delay(delay));
    auto callback = Action::Ptr(new CallBackVoid(func));
    auto action = Action::Ptr(new Sequence({delay_action, callback}));
    this->runAction(action);
}

void Widget::defer(Widget* sender, std::function<void(Widget*)> const& func, float delay) {
    auto delay_action = Action::Ptr(new Delay(delay));
    auto callback = Action::Ptr(new CallBackSender(sender, func));
    auto action = Action::Ptr(new Sequence({delay_action, callback}));
    this->runAction(action);
}

void Widget::enableUpdate(bool update) {
    _update = update;
}

void Widget::setVisible(bool visible) {
    _visible = visible;
    this->onVisible(_visible);
}

void Widget::performLayout() {
    this->modifyLayout();
}

void Widget::addChild(WidgetPtr& widget) {
    if (widget->_parent) {
        widget->_parent->removeChild(widget);
        widget->_parent = nullptr;
    }
    _children.push_back(widget);
    widget->_parent = this;
    widget->enter();
}

void Widget::removeChild(WidgetPtr& widget) {
    this->removeChild(widget.get());
}

void Widget::removeChild(Widget* widget) {
    for (auto iter = _children.begin(); iter != _children.end(); iter++) {
        if (iter->get() == widget) {
            root()->runAction(Action::Ptr(new KeepAlive<Widget>(*iter))); // 下一帧之前保留引用
            widget->exit();
            widget->_parent = nullptr;
            _children.erase(iter);
            return;
        }
    }
}

void Widget::removeAllChildren() {
    for (int i = _children.size()-1; i >= 0; --i) {
        auto& child = _children[i];
        child->exit();
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

Widget::WidgetArray const& Widget::children() const {
    return _children;
}

void Widget::update(float delta) {
    bool update = _visible and _update;
    bool action_update = not _pause_action_when_hidden;
    if (_visible or action_update) {
        _action->update(delta);
    }
    if (_dirty) {
        this->modifyLayout();
        _dirty = false;
    }
    if (not update) {
        return;
    }
    this->onUpdate(delta);
    auto list = _children;
    for (auto child : list) {
        if (child->parent() == nullptr) {
            // child is removed, but KeepAlive.
            continue;
        }
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

void Widget::dirty() {
    onDirty();
}

void Widget::enter() {
    onEnter();
}

void Widget::exit() {
    onExit();
}

Widget* Widget::find(std::string const& name) {
    if (name.empty()) {
        return nullptr;
    }
    if (this->name() == name) {
        return this;
    }
    for (auto& child : _children) {
        if (child->name() == name) {
            return child.get();
        }
    }
    return nullptr;
}

Widget* _findWidget(Widget* widget, std::string const& name) {
    if (widget->name() == name) {
        return widget;
    }
    for (auto& child : widget->children()) {
        auto r = _findWidget(child.get(), name);
        if (r != nullptr) {
            return r;
        }
    }
    return nullptr;
}

Widget* Widget::gfind(std::string const& name) {
    return name.empty() ? nullptr : _findWidget(this, name);
}

void Widget::setName(std::string const& name) {
    _name = name;
}

std::string const& Widget::name() const {
    return _name;
}

void Widget::set_userdata(void* data) {
    _userdata = data;
}

void* Widget::userdata() const {
    return _userdata;
}

void Widget::modifyLayout() {
    _global_size = _size * _scale.self_abs();
    _global_position = (_parent ? _parent->_global_position : Vector2f{0, 0}) + (_position - _global_size * _anchor);
    this->dirty();
    _dirty = false;
    for (auto& child : _children) {
        child->modifyLayout();
    }
}

void Widget::onModifyPosition(Vector2f const& position) {

}

void Widget::setPosition(Vector2f const& position) {
    _dirty = true;
    this->onModifyPosition(_position = position);
}

void Widget::setPosition(float dx, float dy) {
    _position.x = dx;
    _position.y = dy;
    _dirty = true;
    this->onModifyPosition(_position);
}

void Widget::setPositionX(float dx) {
    setPosition(dx, _position.y);
}

void Widget::setPositionY(float dy) {
    setPosition(_position.x, dy);
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
    _dirty = true;
    this->onModifySize(_size = size);
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

Vector2f const& Widget::global_size() const {
    return _global_size;
}

void Widget::onModifyAnchor(Vector2f const& anchor) {

}

void Widget::setAnchor(Vector2f const& anchor) {
    _dirty = true;
    this->onModifyAnchor(_anchor = anchor);
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
    _dirty = true;
    this->onModifyScale(_scale = scale);
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

void Widget::setOpacity(unsigned char opacity) {
    this->onModifyOpacity(_opacity = opacity);
}

unsigned char Widget::opacity() const {
    return _opacity;
}

void Widget::onModifyOpacity(unsigned char opacity) {

}

void Widget::setRotation(float rotation) {
    this->onModifyRotation(_rotation = rotation);
}

float Widget::rotation() const {
    return _rotation;
}

void Widget::onModifyRotation(float rotation) {

}

void Widget::runAction(ActionPtr const& action) {
    _action->add(action);
}

void Widget::stopAction(ActionPtr const& action) {
    _action->remove(action);
}

void Widget::stopAction(std::string const& name) {
    _action->remove(name);
}

bool Widget::hasAction(std::string const& name) const {
    return _action->has(name);
}

void Widget::stopAllActions() {
    _action->clear();
}

void Widget::pauseAllActionWhenHidden(bool yes) {
    _pause_action_when_hidden = yes;
}

void Widget::pauseAllActions() {
    _action->pause(true);
}

void Widget::resumeAllActions() {
    _action->pause(false);
}

//=====================================================================================

WindowWidget::WindowWidget() {
    enableUpdate(true);
}

//=====================================================================================

Widget::Ptr RenderTargetWidget::New(Vector2i const& textureSize) {
    if (SDL_RenderTargetSupported(_game.renderer())) {
        return Ptr(new RenderTargetWidget(textureSize));
    }
    return Ptr(new WindowWidget);
}

RenderTargetWidget::RenderTargetWidget(Vector2i const& textureSize) {
    auto texture = SDL_CreateTexture(
            _game.renderer(),
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            textureSize.x,
            textureSize.y
    );
    _texture = TexturePtr(new Texture(texture));
    if (texture) {
        //some problem on psvita
        //SDL_SetTextureScaleMode(texture, SDL_ScaleModeLinear);
    }
    _target = std::make_shared<RenderCopyEx>();
    _target->setTexture(_texture->data());
}

void RenderTargetWidget::draw(SDL_Renderer* renderer) {
    if (not _visible) {
        return;
    }

    auto scale = _scale;
    auto anchor = _anchor;
    auto position = _position;
    auto parent = _parent;
    this->_scale = {1.0f, 1.0f};
    this->_anchor = {0.0f, 0.0f};
    this->_position = {0.0f, 0.0f};
    this->_parent = nullptr;
    this->modifyLayout();

    auto render_target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, _texture->data());
    SDL_RenderClear(renderer);

    this->onDraw(renderer);
    for (auto& child : _children) {
        child->draw(renderer);
    }
    SDL_SetRenderTarget(renderer, render_target);

    _parent = parent;
    _position = position;
    _anchor = anchor;
    _scale = scale;
    this->modifyLayout();

    _target->setAnchor(_anchor);
    _target->setScale(_scale);
    _target->setSize(_size.to<int>());
    _target->setOpacity(_opacity);
    _target->setAngle(_rotation);
    _target->draw(renderer, _position.to<int>());
}

//=====================================================================================

void GamePadWidget::enter() {
    _game.gamepad().add(this->ptr());
    onEnter();
}

void GamePadWidget::exit() {
    onExit();
    _game.gamepad().remove(this->ptr());
}

void GamePadWidget::sleep_gamepad(float seconds) {
    _game.gamepad().sleep(seconds);
}

//=====================================================================================

ImageWidget::ImageWidget(TexturePtr const& texture):_target(std::make_shared<RenderCopy>()) {
    this->setTexture(texture);
}

ImageWidget::ImageWidget(TexturePtr const& texture, SDL_Rect const& srcrect):_target(std::make_shared<RenderCopy>()) {
    this->setTexture(texture, srcrect);
}

void ImageWidget::setTexture(TexturePtr const& texture) {
    if (texture == nullptr) {
        setSize(0.0f, 0.0f);
        return;
    }
    _texture = texture;
    _target->setTexture(_texture->data());
    this->setSize(_target->size().to<float>());
}

void ImageWidget::setTexture(TexturePtr const& texture, SDL_Rect const& srcrect) {
    if (texture == nullptr) {
        return;
    }
    _texture = texture;
    _target->setTexture(_texture->data(), srcrect);
    this->setSize(_target->size().to<float>());
}

void ImageWidget::dirty() {
    _target->setSize(_global_size.x, _global_size.y);
    this->onDirty();
}

void ImageWidget::onModifyOpacity(unsigned char opacity) {
    _target->setOpacity(opacity);
}

void ImageWidget::onDraw(SDL_Renderer* renderer) {
    _target->draw(renderer, global_position().to<int>());
}

//=====================================================================================

ButtonWidget::ButtonWidget(TexturePtr const& normal, TexturePtr const& pressed, TexturePtr const& disabled):
ImageWidget(normal),
_state(NORMAL),
_enable(true) {
    setNormalTexture(normal);
    setPressedTexture(pressed);
    setDisabledTexture(disabled);
}

void ButtonWidget::setNormalTexture(TexturePtr const& normal) {
    _texture[NORMAL] = normal;
}

void ButtonWidget::setPressedTexture(TexturePtr const& pressed) {
    _texture[PRESSED] = pressed;
}

void ButtonWidget::setDisabledTexture(TexturePtr const& disabled) {
    _texture[DISABLED] = disabled;
}

void ButtonWidget::setEnable(bool enable) {
    setState((_enable = enable) ? NORMAL : DISABLED);
}

void ButtonWidget::setPressed(bool pressed) {
    if (not _enable) {
        return;
    }
    setState(pressed ? PRESSED : NORMAL);
}

bool ButtonWidget::enable() const {
    return _enable;
}

bool ButtonWidget::pressed() const {
    return _state == PRESSED;
}

void ButtonWidget::setClick(CallBack const& cb) {
    _callback = cb;
}

void ButtonWidget::click() {
    if (_enable and _callback != nullptr) {
        _callback();
    }
}

void ButtonWidget::setState(State state) {
    if (_state == state) {
        return;
    }
    setTexture(_texture[state]);
    _state = state;
}

//=====================================================================================

MaskWidget::MaskWidget(SDL_Color const& c):_color(c) {
    setOpacity(c.a);
}

void MaskWidget::setColor(SDL_Color const& c) {
    _color = c;
    setOpacity(c.a);
}

SDL_Color const& MaskWidget::color() const {
    return _color;
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
    _color.a = _opacity;
    dc.setColor(_color);
    SDL_RenderFillRectF(renderer, &dst);
}

//=====================================================================================

CurtainWidget::CurtainWidget(SDL_Color const& c) {
    Vector2f anchor[2] = {
            {0.5f, 1.0f},
            {0.5f, 0.0f},
    };
    for (int i = 0; i < 2; ++i) {
        addChild(_mask[i]= Widget::Ptr(new MaskWidget(c)));
        _mask[i]->setVisible(false);
        _mask[i]->setAnchor(anchor[i]);
    }
    enableUpdate(true);
}

void CurtainWidget::setState(State s) {
    if (s == ON) {
        for (int i = 0; i < 2; ++i) {
            _mask[i]->setPosition(size().x * 0.5f, size().y * 0.5f);
            _mask[i]->setVisible(true);
        }
    } else {
        Vector2f position[2] = {
                {size().x * 0.5f, 0.0f},
                {size().x * 0.5f, size().y},
        };
        for (int i = 0; i < 2; ++i) {
            _mask[i]->setPosition(position[i]);
            _mask[i]->setVisible(false);
        }
    }
}

void CurtainWidget::fadeIn(float duration) {
    Vector2f position[2] = {
            {size().x * 0.5f, 0.0f},
            {size().x * 0.5f, size().y},
    };
    float step[2] = {size().y * 0.5f, size().y * -0.5f};
    for (int i = 0; i < 2; ++i) {
        _mask[i]->setVisible(true);
        _mask[i]->setPosition(position[i]);
        moveMaskVertical(_mask[i]->to<MaskWidget>(), step[i], duration);
    }
}

void CurtainWidget::fadeOut(float duration) {
    float step[2] = {size().y * -0.5f, size().y * 0.5f};
    for (int i = 0; i < 2; ++i) {
        _mask[i]->setVisible(true);
        _mask[i]->setPosition(size().x * 0.5f, size().y * 0.5f);
        moveMaskVertical(_mask[i]->to<MaskWidget>(), step[i], duration);
    }
}

void CurtainWidget::moveMaskVertical(MaskWidget* target, float yStep, float duration) {
    auto move = Action::Ptr(new MoveBy(target, {0, yStep}, duration));
    target->runAction(move);
}

//=====================================================================================

ScreenWidget::ScreenWidget():_curtain(nullptr), _root(nullptr) {

    Widget::Ptr box(new WindowWidget);
    Widget::Ptr window(new WindowWidget);
    Widget::Ptr curtain(new CurtainWidget);

    addChild(box);
    box->addChild(window);
    box->addChild(curtain);

    _root = window->to<WindowWidget>();
    _curtain = curtain->to<CurtainWidget>();
    _action = ActionExecuterPtr(new SafeActionExecuter); // make action executer thread safe
}

void ScreenWidget::push(Widget::Ptr& widget) {
    _root->addChild(widget);
    for (auto& child : _root->children()) {
        if (child.get() == widget.get()) {
            return;
        }
        child->setVisible(false);
    }
}

void ScreenWidget::replace(Widget::Ptr& widget) {
    this->pop();
    this->push(widget);
}

void ScreenWidget::pop() {
    auto& c = _root->children();
    if (c.size()) {
        _root->removeChild(c.back());
    }
    if (c.size()) {
        c.back()->setVisible(true);
    }
}

void ScreenWidget::popAll() {
    _root->removeAllChildren();
}

void ScreenWidget::update(float delta) {
    WindowWidget::update(delta);
}

void ScreenWidget::render(SDL_Renderer* renderer) {
    WindowWidget::draw(renderer);
}

int ScreenWidget::scene_size() const {
    return _root->children().size();
}

Widget::Ptr& ScreenWidget::scene_at(int index) const {
    return _root->children()[index];
}

Widget::Ptr& ScreenWidget::scene_back() const {
    return scene_at(scene_size()-1);
}

Widget::Ptr ScreenWidget::find(std::string const& name) const {
    for (auto& child : _root->children()) {
        if (child->name() == name) {
            return child;
        }
    }
    return nullptr;
}

void ScreenWidget::runAction(Action::Ptr const& action) {
    WindowWidget::runAction(action);
}

void ScreenWidget::stopAction(Action::Ptr const& action) {
    WindowWidget::stopAction(action);
}

void ScreenWidget::stopAction(std::string const& name) {
    WindowWidget::stopAction(name);
}

//=====================================================================================

TTFLabel::TTFLabel():ImageWidget(nullptr) {}

TTFLabel::Ptr TTFLabel::New(std::string const& text, TTFontPtr const& font, Vector2f const& anchor) {
    auto label = new TTFLabel;
    label->setFont(font);
    label->setString(text);
    label->setAnchor(anchor);
    return Ptr(label);
}

void TTFLabel::setFont(TTFontPtr const& font) {
    _font = font;
}

TTFont::Ptr const& TTFLabel::font() const {
    return _font;
}

void TTFLabel::setString(std::string const& s) {
    if (_font == nullptr) {
        return;
    }
    setTexture(_font->createWithUTF8(_game.renderer(), s.c_str()));
}

void TTFLabel::setString(std::string const& s, SDL_Color const& color) {
    if (_font != nullptr) {
        _font->setColor(color);
    }
    setString(s);
}

//=====================================================================================

FrameAnimationWidget::FrameAnimationWidget():
ImageWidget(nullptr),
_index(0),
_frame_tick(0.0f),
_frame_time(0.0f),
_loop(false) {
    enableUpdate(true);
}

void FrameAnimationWidget::setFrames(FrameArray const& frames) {
    _frames = frames;
    if (_frames.size()) {
        setTexture(_frames[0]);
    }
}

void FrameAnimationWidget::play(float duration, bool loop) {
    if (_frames.empty()) {
        return;;
    }
    _loop = loop;
    _frame_time = duration / _frames.size();
    _frame_tick = 0.0f;
    _index = 0;
    setTexture(_frames[0]);
    startAnimate();
}

void FrameAnimationWidget::play_once(float duration) {
    this->play(duration, false);
}

void FrameAnimationWidget::stop() {
    if (_frames.empty()) {
        return;;
    }
    setTexture(_frames[0]);
    stopAction("animate");
}

void FrameAnimationWidget::startAnimate() {
    if (hasAction("animate")) {
        return;
    }
    auto call = Action::Ptr(new CallBackDelta(std::bind(&FrameAnimationWidget::onAnimate, this, std::placeholders::_1)));
    auto action = Action::New<Repeat>(call);
    action->setName("animate");
    runAction(action);
}

void FrameAnimationWidget::onAnimate(float delta) {
    bool modify = false;
    while ((_frame_tick += delta) >= _frame_time) {
        _frame_tick -= _frame_time;
        if (++_index >= _frames.size()) {
            if (_loop) {
                _index = 0;
            } else {
                _index = _frames.size() - 1;
                stopAction("animate");
            }
        }
        modify = true;
    }
    if (modify) {
        setTexture(_frames[_index]);
    }
}

mge_end
