//
// Created by baifeng on 2021/10/7.
//

#ifndef SDL2_UI_BATTLE_H
#define SDL2_UI_BATTLE_H

#include "common/widget.h"
#include "common/quadtree.h"
#include <list>

class TankView;
class TileModel;
class WorldModel;
class BattleFieldView : public GamePadWidget {
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
    void onEvent(Event const& e) override;
    void addElement(Widget::Ptr& widget);
    void procTankControl();
    void gameOver();
    void pause(bool v);
private:
    void add_key(int key);
    bool remove_key(int key);
private:
    bool _pause;
    Widget* _floor;
    Widget* _root;
    Widget* _upper;
    TankView* _player;
    std::list<int> _keylist;
    WorldModelPtr _world;
};


#endif //SDL2_UI_BATTLE_H
