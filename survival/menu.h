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

#include "dungeon/generate.h"
#include "dungeon/Irregular_maze_of_large_and_small_rooms.h"

class RandomRoomView : public mge::GamePadWidget {
public:
    RandomRoomView();
    ~RandomRoomView();
private:
    void onUpdate(float delta) override;
    void draw(SDL_Renderer* renderer) override;
    void onButtonDown(int key) override;
    void updateRoomLayout();
private:
    int _step;
    mge::Widget* _window;
    dungeon::Builder* _builder;
    dungeon::lasr::Data* _data;
};

#endif //SDL2_UI_MENU_H
