//
// Created by baifeng on 2021/11/16.
//

#ifndef SDL2_UI_MENU_H
#define SDL2_UI_MENU_H

#include "common/widget.h"

class WeaponListView;
class WeaponSelectView : public mge::GamePadWidget {
public:
    WeaponSelectView();
private:
    void onButtonDown(int key) override;
    void onButtonUp(int key) override;
private:
    WeaponListView* _weaponView;
};

class WorldTileMap;
class BattleWorldView : public mge::GamePadWidget {
public:
    BattleWorldView();
private:
    void onButtonDown(int key) override;
    void onButtonUp(int key) override;
private:
    WorldTileMap* _worldMap;
};

#endif //SDL2_UI_MENU_H
