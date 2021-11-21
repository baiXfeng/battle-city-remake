//
// Created by baifeng on 2021/9/24.
//

#ifndef SDL2_UI_WIDGET_H
#define SDL2_UI_WIDGET_H

#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>
#include <string>
#include <SDL.h>
#include "types.h"
#include "gamepad.h"
#include "mouse.h"
#include "event.h"
#include "observer.h"

mge_begin

void TestWidget();

class Action;
class BaseActionExecuter;
class Widget : public GamePadListener, public Event::Listener {
public:
    typedef std::shared_ptr<Widget> WidgetPtr;
    typedef std::vector<WidgetPtr> WidgetArray;
    typedef std::shared_ptr<BaseActionExecuter> ActionExecuterPtr;
    typedef std::shared_ptr<Action> ActionPtr;
    typedef WidgetPtr Ptr;
    enum EVENT {
        ON_ENTER = 0xABEF0001,
        ON_EXIT,
    };
    typedef Signal<void(Widget*)> SenderSignal;
    typedef std::unordered_map<int, SenderSignal> SignalPool;
public:
    template<typename T, typename... Args>
    static Ptr New(Args const&... args) {
        return Ptr(new T(args...));
    }
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
    void defer(std::function<void()> const& func, float delay);
    void defer(Widget* sender, std::function<void(Widget*)> const& func, float delay);
public:
    void enableUpdate(bool update);
    void enableClip(bool clip);
    void setVisible(bool visible);
    void performLayout();
public:
    virtual void addChild(WidgetPtr const& widget);
    virtual void addChild(WidgetPtr const& widget, int index);
    virtual void removeChild(WidgetPtr const& widget);
    virtual void removeChild(Widget* widget);
    virtual void removeAllChildren();
    virtual void removeFromParent();
    WidgetArray& children();
    WidgetArray const& children() const;
public:
    SenderSignal::slot_type connect(int type, SenderSignal::observer_type const& obs);
    void disconnect(int type, SenderSignal::slot_type const& obs);
public:
    virtual void update(float delta);
    virtual void draw(SDL_Renderer* renderer);
protected:
    virtual void dirty();
    virtual void onUpdate(float delta) {}
    virtual void onDraw(SDL_Renderer* renderer) {}
    virtual void onDirty() {}
    virtual void onVisible(bool visible) {}
protected:
    virtual void enter();
    virtual void exit();
    virtual void onEnter() {}
    virtual void onExit() {}
public:
    Widget* find(std::string const& name);
    Widget* gfind(std::string const& name);
    void setName(std::string const& name);
    std::string const& name() const;
public:
    void set_userdata(void* data);
    void* userdata() const;
public:
    void setPosition(Vector2f const& position);
    void setPosition(float dx, float dy);
    void setPositionX(float dx);
    void setPositionY(float dy);
    Vector2f const& position() const;
    Vector2f const& global_position() const;
    virtual void onModifyPosition(Vector2f const& position);
public:
    void setSize(Vector2f const& size);
    void setSize(float sx, float sy);
    Vector2f const& size() const;
    Vector2f const& global_size() const;
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
    void setOpacity(unsigned char opacity);
    unsigned char opacity() const;
    virtual void onModifyOpacity(unsigned char opacity);
public:
    void setRotation(float rotation);
    float rotation() const;
    virtual void onModifyRotation(float rotation);
public:
    void runAction(ActionPtr const& action);
    void stopAction(ActionPtr const& action);
    void stopAction(std::string const& name);
    bool hasAction(std::string const& name) const;
    void stopAllActions();
    void pauseAllActionWhenHidden(bool yes = true);
    void pauseAllActions();
    void resumeAllActions();
protected:
    void modifyLayout();
    SenderSignal& signal(int key);
protected:
    bool _visible;
    bool _update;
    bool _clip;
    bool _pause_action_when_hidden;
    bool _dirty;
    unsigned char _opacity;
    void* _userdata;
    Widget* _parent;
    float _rotation;
    Vector2f _position;
    Vector2f _global_position;
    Vector2f _global_size;
    Vector2f _size;
    Vector2f _scale;
    Vector2f _anchor;
    std::string _name;
    WidgetArray _children;
    ActionExecuterPtr _action;
    SignalPool _signal_pool;
};

class WindowWidget : public Widget {
public:
    WindowWidget();
};

class Texture;
class RenderCopyEx;
class RenderTargetWidget : public WindowWidget {
public:
    typedef std::shared_ptr<Texture> TexturePtr;
    typedef RenderCopyEx Render;
    typedef std::shared_ptr<Render> RenderPtr;
public:
    void setRenderTargetSize(Vector2i const& size);
    void setRenderTargetNull();
protected:
    RenderTargetWidget();
    void draw(SDL_Renderer* renderer) override;
    void drawRenderTarget(SDL_Renderer* renderer);
protected:
    bool _hasRender;
    RenderPtr _render;
};

class GamePadWidget : public WindowWidget {
public:
    typedef GamePad::KeyCode KeyCode;
public:
    GamePadWidget();
protected:
    void sleep_gamepad(float seconds);
};

class RenderCopyEx;
class ImageWidget : public Widget {
public:
    typedef RenderCopyEx Render;
    typedef std::shared_ptr<Render> RenderPtr;
    typedef std::shared_ptr<Texture> TexturePtr;
public:
    ImageWidget(TexturePtr const& texture);
    ImageWidget(TexturePtr const& texture, SDL_Rect const& srcrect);
public:
    void setTexture(TexturePtr const& texture);
    void setTexture(TexturePtr const& texture, SDL_Rect const& srcrect);
protected:
    void onDraw(SDL_Renderer* renderer) override;
    void onModifyOpacity(unsigned char opacity) override;
    void onModifyRotation(float rotation) override;
    void onModifySize(Vector2f const& size) override;
    void onModifyScale(Vector2f const& scale) override;
    void onModifyAnchor(Vector2f const& anchor) override;
private:
    RenderPtr _target;
};

class ButtonWidget : public ImageWidget {
public:
    typedef std::function<void()> CallBack;
    typedef std::shared_ptr<Texture> TexturePtr;
    enum State {
        NORMAL = 0,
        PRESSED,
        DISABLED,
    };
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
    void addChild(WidgetPtr const& widget) override {}
    void setState(State state);
private:
    bool _enable;
    State _state;
    CallBack _callback;
    TexturePtr _texture[3];
};

class MaskWidget : public Widget {
public:
    MaskWidget(SDL_Color const& c);
public:
    void setColor(SDL_Color const& c);
    SDL_Color const& color() const;
private:
    void onDraw(SDL_Renderer* renderer) override;
private:
    SDL_Color _color;
};

class CurtainWidget : public Widget {
public:
    typedef std::shared_ptr<Action> ActionPtr;
    typedef std::function<void(Widget*)> CallFunc;
    enum State {
        ON = 0,
        OFF,
    };
public:
    CurtainWidget(SDL_Color const& c = {0, 0, 0, 255});
public:
    void setState(State s);
    void fadeIn(float duration);
    void fadeOut(float duration);
private:
    void moveMaskVertical(MaskWidget* target, float yStep, float duration);
private:
    float _duration;
    Widget::Ptr _mask[2];
};

class ScreenWidget : protected WindowWidget {
    friend class Game;
public:
    ScreenWidget();
public:
    void push(Widget::Ptr& widget);
    void replace(Widget::Ptr& widget);
    void pop();
    void popAll();
    template <typename T, typename... Args> void push(Args const&... args) {
        Widget::Ptr widget(new T(args...));
        this->push(widget);
    }
    template <typename T, typename... Args> void replace(Args const&... args) {
        this->pop();
        this->template push<T>(args...);
    }
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
private:
    CurtainWidget* _curtain;
    WindowWidget* _root;
};

class TTFont;
class TTFLabel : public ImageWidget {
public:
    typedef std::shared_ptr<TTFont> TTFontPtr;
public:
    static Ptr New(std::string const& text, TTFontPtr const& font, Vector2f const& anchor = {0.0f, 0.0f});
    TTFLabel();
public:
    void setFont(TTFontPtr const& font);
    TTFontPtr const& font() const;
public:
    void setString(std::string const& s);
    void setString(std::string const& s, SDL_Color const& color);
    std::string const& str() const;
private:
    TTFontPtr _font;
    std::string _s;
};

class FrameAnimationWidget : public ImageWidget {
public:
    typedef std::vector<TexturePtr> FrameArray;
public:
    FrameAnimationWidget();
public:
    void setFrames(FrameArray const& frames);
    void play(float duration, bool loop = true);
    void play_once(float duration);
    void stop();
private:
    void startAnimate();
    void onAnimate(float delta);
private:
    bool _loop;
    int _index;
    float _frame_tick;
    float _frame_time;
    FrameArray _frames;
};

mge_end

#endif //SDL2_UI_WIDGET_H
