//
// Created by baifeng on 2021/9/29.
//

#ifndef BATTLE_CITY_VIEW_H
#define BATTLE_CITY_VIEW_H

#include "common/widget.h"
#include "common/quadtree.h"
#include "data.h"

namespace mge {
    class MaskWidget;
}
class LogoView : public mge::GamePadWidget {
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
    mge::MaskWidget* _mask;
    Callback _callback;
};

class StartView : public mge::GamePadWidget {
public:
    StartView();
private:
    void onEnter() override;
    void onButtonDown(int key) override;
    void onStart(int index);
private:
    bool _canSelect;
    int _index;
    std::vector<mge::Vector2f> _position;
};

class SelectLevelView : public mge::GamePadWidget {
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
    mge::TTFLabel* _label;
    mge::CurtainWidget* _curtain;
    float _duration;
};

class BattleView : public mge::WindowWidget {
public:
    BattleView();
    ~BattleView();
private:
    void onEvent(mge::Event const& e) override;
};

class BattleInfoView : public mge::Widget {
public:
    BattleInfoView();
    ~BattleInfoView();
private:
    void onEvent(mge::Event const& e) override;
    void onEnemyNumberChanged(int n);
    void onPlayerNumberChanged(int n);
private:
    mge::ImageWidget* createEnemyIcon();
    mge::TTFLabel* _playerLife;
    Widget::Ptr _enemy[20];
};

class ScoreView : public mge::WindowWidget {
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
    mge::TTFLabel* _total;
};

class GameOverView : public mge::GamePadWidget {
public:
    GameOverView();
private:
    void onButtonDown(int key) override;
};

class CheatView : public mge::GamePadWidget {
public:
    typedef std::function<void()> CallBack;
    enum {
        TITLE_MAX = 2,
    };
public:
    CheatView(CallBack const& cb = nullptr);
private:
    void onButtonDown(int key) override;
private:
    int _index;
    Ptr _icon;
    mge::Vector2f _position[TITLE_MAX];
    CallBack _callback;
};

class CheatListView : public mge::GamePadWidget {
public:
    enum {
        TITLE_MAX = 4,
    };
public:
    CheatListView();
private:
    void onButtonDown(int key) override;
    void onCheat(int index);
private:
    int _index;
    Ptr _icon;
    mge::Vector2f _position[TITLE_MAX];
    std::vector<mge::TTFLabel*> _labels;
};

class BigExplosionView : public mge::FrameAnimationWidget {
public:
    typedef std::function<void()> FinishCall;
public:
    BigExplosionView();
    void play(FinishCall const& fc = nullptr);
};

class BulletExplosionView : public mge::FrameAnimationWidget {
public:
    BulletExplosionView();
    void play();
};

class BattleFieldInterface {
public:
    virtual ~BattleFieldInterface() {}
public:
    virtual void addToBottom(mge::Widget::Ptr& widget) {}
    virtual void addToMiddle(mge::Widget::Ptr& widget) {}
    virtual void addToTop(mge::Widget::Ptr& widget) {}
};

#endif //BATTLE_CITY_VIEW_H
