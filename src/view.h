//
// Created by baifeng on 2021/9/29.
//

#ifndef SDL2_UI_VIEW_H
#define SDL2_UI_VIEW_H

#include "common/widget.h"
#include "common/quadtree.h"
#include "data.h"

class MaskWidget;
class LogoView : public GamePadWidget {
    typedef std::function<void()> Callback;
public:
    LogoView();
    void setFinishCall(Callback const& cb);
private:
    void onFadeIn(float v);
    void onFadeOut(float v);
    void onEnter() override;
    void onButtonDown(int key) override;
private:
    bool _canClick;
    MaskWidget* _mask;
    Callback _callback;
};

class StartView : public GamePadWidget {
public:
    StartView();
private:
    void onEnter() override;
    void onButtonDown(int key) override;
    void onStart(int index);
private:
    bool _canSelect;
    int _index;
    std::vector<Vector2f> _position;
};

class SelectLevelView : public GamePadWidget {
public:
    SelectLevelView();
private:
    void onButtonDown(int key) override;
    void onButtonUp(int key) override;
    void addLevel();
    void subLevel();
    void autoAddLevel(bool add);
    void stopAutoAddLevel();
private:
    int _level;
    TTFLabel* _label;
    CurtainWidget* _curtain;
    float _duration;
};

class BattleView : public WindowWidget {
public:
    BattleView();
};

class BattleInfoView : public Widget {
public:
    BattleInfoView();
    ~BattleInfoView();
private:
    void onEvent(Event const& e) override;
    void onEnemyNumberChanged(int n);
    void onPlayerNumberChanged(int n);
private:
    ImageWidget* createEnemyIcon();
    TTFLabel* _playerLife;
    Widget::Ptr _enemy[20];
};

class ScoreView : public WindowWidget {
public:
    enum Alignment {
        LEFT = 0,
        RIGHT,
        MIDDLE,
    };
    ScoreView();
private:
    void showTotal();
    void onNextScene();
private:
    TTFLabel* _total;
};

class GameOverView : public GamePadWidget {
public:
    GameOverView();
private:
    void onButtonDown(int key) override;
};

class CheatView : public GamePadWidget {
public:
    typedef std::function<void()> CallBack;
public:
    CheatView(CallBack const& cb = nullptr);
private:
    void onButtonDown(int key) override;
private:
    int _index;
    Ptr _icon;
    Vector2f _position[2];
    CallBack _callback;
};

class BigExplosionView : public FrameAnimationWidget {
public:
    typedef std::function<void()> FinishCall;
public:
    BigExplosionView();
    void play(FinishCall const& fc);
};

class BulletExplosionView : public FrameAnimationWidget {
public:
    BulletExplosionView();
    void play();
};

class BattleFieldInterface {
public:
    virtual ~BattleFieldInterface() {}
public:
    virtual void addToBottom(Widget::Ptr& widget) {}
    virtual void addToMiddle(Widget::Ptr& widget) {}
    virtual void addToTop(Widget::Ptr& widget) {}
};

#endif //SDL2_UI_VIEW_H
