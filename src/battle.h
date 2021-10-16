//
// Created by baifeng on 2021/10/7.
//

#ifndef SDL2_UI_BATTLE_H
#define SDL2_UI_BATTLE_H

#include "common/widget.h"
#include "common/quadtree.h"
#include "behavior.h"
#include "view.h"
#include <list>

class TankView;
class TileModel;
class WorldModel;
class BattleFieldView : public GamePadWidget, public BattleFieldInterface {
    typedef std::shared_ptr<WorldModel> WorldModelPtr;
public:
    BattleFieldView();
    ~BattleFieldView();
private:
    void onLoadLevel();
    void onUpdate(float delta) override;
    void draw(SDL_Renderer* renderer) override;
    void onDraw(SDL_Renderer* renderer) override;
    void onButtonDown(int key) override;
    void onButtonUp(int key) override;
    void onJoyAxisMotion(JOYIDX joy_id, int x, int y) override;
    void onEvent(Event const& e) override;
    void procTankControl();
    void gameOver();
    void pause(bool v);
private:
    void addToBottom(Widget::Ptr& widget) override;
    void addToMiddle(Widget::Ptr& widget) override;
    void addToTop(Widget::Ptr& widget) override;
private:
    void add_key(int key);
    bool remove_key(int key);
private:
    bool _pause;
    bool _joyUsed;
    Widget* _floor;
    Widget* _root;
    Widget* _upper;
    TankView* _player;
    std::list<int> _keylist;
    WorldModel* _world;
    Behavior::Ptr _behavior;
};


#endif //SDL2_UI_BATTLE_H
