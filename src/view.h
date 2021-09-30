//
// Created by baifeng on 2021/9/29.
//

#ifndef SDL2_UI_VIEW_H
#define SDL2_UI_VIEW_H

#include "common/widget.h"

Widget::Ptr firstScene();

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

#endif //SDL2_UI_VIEW_H
