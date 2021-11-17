//
// Created by baifeng on 2021/11/16.
//

#ifndef SDL2_UI_MENU_H
#define SDL2_UI_MENU_H

#include "common/widget.h"

class testTable;
class WeaponSelectWidget : public mge::GamePadWidget {
public:
    WeaponSelectWidget();
private:
    void onButtonDown(int key) override;
    void onButtonUp(int key) override;
private:
    testTable* _tableView;
};

#endif //SDL2_UI_MENU_H
