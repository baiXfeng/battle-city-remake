//
// Created by baifeng on 2021/9/24.
//

#ifndef SDL2_UI_VIEW_H
#define SDL2_UI_VIEW_H

#include <memory>
#include <vector>
#include <functional>
#include <string>
#include <SDL.h>
#include "types.h"
#include "gamepad.h"

void TestWidget();

class Action;
class ActionExecuter;
class Widget : public GamePadListener {
public:
    typedef std::shared_ptr<Widget> WidgetPtr;
    typedef std::vector<WidgetPtr> WidgetArray;
    typedef std::shared_ptr<ActionExecuter> ActionExecuterPtr;
    typedef std::shared_ptr<Action> ActionPtr;
    typedef WidgetPtr Ptr;
public:
    Widget();
    virtual ~Widget();
public:
    Widget* root();
    Widget::Ptr ptr() const;
    Widget* parent() const;
    bool visible() const;
    template<class T> T* to() {
        return dynamic_cast<T*>(this);
    }
    void defer(std::function<void()> const& func, float delay = 0.0f);
    void defer(Widget* sender, std::function<void(Widget*)> const& func, float delay = 0.0f);
public:
    void enableUpdate(bool update);
    void setVisible(bool visible);
public:
    virtual void addChild(WidgetPtr& widget);
    virtual void removeChild(WidgetPtr& widget);
    virtual void removeChild(Widget* widget);
    virtual void removeAllChildren();
    virtual void removeFromParent();
    WidgetArray& children();
    WidgetArray const& children() const;
public:
    virtual void update(float delta);
    virtual void draw(SDL_Renderer* renderer);
    virtual void onUpdate(float delta);
    virtual void onDraw(SDL_Renderer* renderer);
    virtual void onDirty();
public:
    Widget* find(std::string const& name);
    Widget* gfind(std::string const& name);
    void setName(std::string const& name);
    std::string const& name() const;
public:
    void setPosition(Vector2f const& position);
    void setPosition(float dx, float dy);
    Vector2f const& position() const;
    Vector2f const& global_position() const;
    virtual void onModifyPosition(Vector2f const& position);
public:
    void setSize(Vector2f const& size);
    void setSize(float sx, float sy);
    Vector2f const& size() const;
    virtual void onModifySize(Vector2f const& size);
public:
    void setAnchor(Vector2f const& anchor);
    void setAnchor(float x, float y);
    Vector2f const& anchor() const;
    virtual void onModifyAnchor(Vector2f const& anchor);
public:
    void setScale(Vector2f const& scale);
    void setScale(float sx, float sy);
    Vector2f const& scale() const;
    virtual void onModifyScale(Vector2f const& scale);
public:
    void runAction(ActionPtr const& action);
    void stopAction(ActionPtr const& action);
    void stopAction(std::string const& name);
    void stopAllActions();
    void pauseAllActionWhenHidden(bool yes = true);
protected:
    void modifyPosition();
    virtual void onEnter() {}
    virtual void onExit() {}
protected:
    bool _visible;
    bool _update;
    bool _pause_action_when_hidden;
    bool _dirty;
    Widget* _parent;
    Vector2f _position;
    Vector2f _global_position;
    Vector2f _size;
    Vector2f _scale;
    Vector2f _anchor;
    std::string _name;
    WidgetArray _children;
    ActionExecuterPtr _action;
};

class WindowWidget : public Widget {
public:
    WindowWidget();
};

class GamePadWidget : public WindowWidget {
protected:
    void onEnter() override;
    void onExit() override;
    void gamepad_sleep(float seconds);
};

class RenderCopy;
class Texture;
class ImageWidget : public Widget {
public:
    typedef std::shared_ptr<RenderCopy> RenderCopyPtr;
    typedef std::shared_ptr<Texture> TexturePtr;
public:
    ImageWidget(TexturePtr const& texture);
    ImageWidget(TexturePtr const& texture, SDL_Rect const& srcrect);
public:
    void setTexture(TexturePtr const& texture);
    void setTexture(TexturePtr const& texture, SDL_Rect const& srcrect);
private:
    void onDraw(SDL_Renderer* renderer) override;
    void onDirty() override;
private:
    RenderCopyPtr _target;
    TexturePtr _texture;
};

class ButtonWidget : public Widget {
public:
    typedef std::function<void()> CallBack;
    typedef std::shared_ptr<Texture> TexturePtr;
public:
    ButtonWidget(TexturePtr const& normal, TexturePtr const& pressed, TexturePtr const& disabled = nullptr);
public:
    void setNormalTexture(TexturePtr const& normal);
    void setPressedTexture(TexturePtr const& pressed);
    void setDisabledTexture(TexturePtr const& disabled);
public:
    void setEnable(bool enable);
    void setPressed(bool pressed);
    bool enable() const;
    bool pressed() const;
    void setClick(CallBack const& cb);
    void click();
private:
    void addChild(WidgetPtr& widget) override {}
private:
    bool _enable;
    CallBack _callback;
};

class DrawColor {
public:
    DrawColor(SDL_Renderer* r):_renderer(r) {
        SDL_GetRenderDrawColor(_renderer, &_back.r, &_back.g, &_back.b, &_back.a);
    }
    ~DrawColor() {
        SDL_SetRenderDrawColor(_renderer, _back.r, _back.g, _back.b, _back.a);
    }
    void setColor(SDL_Color const& c) {
        SDL_SetRenderDrawColor(_renderer, c.r, c.g, c.b, c.a);
    }
private:
    SDL_Renderer* _renderer;
    SDL_Color _back;
};

class MaskWidget : public Widget {
public:
    MaskWidget(SDL_Color const& c);
private:
    void onDraw(SDL_Renderer* renderer) override;
private:
    SDL_Color _color;
};

class CurtainWidget : public Widget {
public:
    typedef std::shared_ptr<Action> ActionPtr;
    typedef std::function<void(Widget*)> CallFunc;
public:
    CurtainWidget(SDL_Color const& c = {0, 0, 0, 255});
    void Start(ActionPtr const& deploy, float duration, ActionPtr const& complete);
private:
    void Close();
    void Open();
private:
    float _duration;
    Widget::Ptr _mask[2];
    ActionPtr _action[2];
};

class ScreenWidget : protected WindowWidget {
    friend class Game;
public:
    ScreenWidget();
public:
    void push(Widget::Ptr& widget);
    void replace(Widget::Ptr& widget);
    void pop();
    template <typename T, typename... Args> void push(Args const&... args) {
        Widget::Ptr widget(new T(args...));
        this->push(widget);
    }
    template <typename T, typename... Args> void replace(Args const&... args) {
        this->pop();
        this->template push<T>(args...);
    }
    void cut_to(ActionPtr const& deploy, float duration = 0.33f, ActionPtr const& complete = nullptr);
public:
    void update(float delta);
    void render(SDL_Renderer* renderer);
public:
    int scene_size() const;
    WidgetPtr& scene_at(int index) const;
    WidgetPtr& scene_back() const;
    WidgetPtr find(std::string const& name) const;
public:
    void runAction(ActionPtr const& action);
    void stopAction(ActionPtr const& action);
    void stopAction(std::string const& name);
public:
    void onEvent(SDL_Event& event);
private:
    CurtainWidget* _curtain;
    WindowWidget* _root;
};

#endif //SDL2_UI_VIEW_H
