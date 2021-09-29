//
// Created by baifeng on 2021/9/29.
//

#ifndef SDL2_UI_VIEW_H
#define SDL2_UI_VIEW_H

#include "common/widget.h"

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

#endif //SDL2_UI_VIEW_H
