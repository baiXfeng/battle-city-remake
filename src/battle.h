//
// Created by baifeng on 2021/10/7.
//

#ifndef SDL2_UI_BATTLE_H
#define SDL2_UI_BATTLE_H

#include "common/widget.h"
#include "common/quadtree.h"
#include <list>

class TankView;
class BattleFieldView : public GamePadWidget {
    typedef DebugQuadTree<Widget::Ptr> DebugQuadTreeT;
public:
    BattleFieldView();
private:
    void onLoadLevel();
    void onUpdate(float delta) override;
    void draw(SDL_Renderer* renderer) override;
    void onDraw(SDL_Renderer* renderer) override;
    void onButtonDown(int key) override;
    void onButtonUp(int key) override;
    void addElement(Widget::Ptr& widget);
    void sortElements();
    void procTankControl();
    void gameOver();
    void pause(bool v);
private:
    void onTankUpdateQuadTree(Widget::Ptr const& tank);
    void onTankMoveCollision(TankView* tank);
private:
    void add_key(int key);
    bool remove_key(int key);
private:
    typedef DebugQuadTreeT WidgetQuadTree;
    typedef std::shared_ptr<WidgetQuadTree> QuadTreePtr;
    bool _pause;
    Widget* _root;
    TankView* _player;
    QuadTreePtr _quadtree;
    std::list<int> _keylist;
    WidgetQuadTree::SquareList _checklist;
};


#endif //SDL2_UI_BATTLE_H
