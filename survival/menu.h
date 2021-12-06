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

class b2World;
class b2Body;
class PhysicsView : public mge::GamePadWidget {
public:
    PhysicsView();
private:
    void addBox(mge::RectI const& r);
    void addPlatform();
    void reset();
private:
    void onUpdate(float delta) override;
    void onDraw(SDL_Renderer* renderer) override;
    void onButtonDown(int key) override;
private:
    b2World* _world;
    std::map<void*, SDL_Rect> _rects;
    std::list<b2Body*> _bodies;
    SDL_Texture* _box;
    SDL_Rect _platform;
};

class RandomRoomView : public mge::GamePadWidget {
public:
    RandomRoomView();
    ~RandomRoomView();
private:
    void rebuild();
    void queryMainRoom();
    void makeGraph();
    void makeMiniSpanTree();
    void addSomeEdge();
private:
    void GenRoom(int room_size, mge::Vector2i const& min_size, mge::Vector2i const& max_size, bool check_overlap = false);
private:
    void onUpdate(float delta) override;
    void onDraw(SDL_Renderer* renderer) override;
    void onButtonDown(int key) override;
    void addRoom(mge::RectI const& r);
    bool isRoomOverlap(mge::RectI const& r) const;
    int getTileSize() const;
private:
    int _step;
    b2World* _world;
    mge::Grid<char> _grid;
    std::vector<mge::RectI> _room;
    std::vector<mge::RectI*> _mainRoom;
    std::map<b2Body*, int> _roomIdx;
    std::vector<std::vector<mge::Vector2f>> _edges;
};

#endif //SDL2_UI_MENU_H
