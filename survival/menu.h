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

#include "dungeon/mini_span_tree.h"
#include "common/observer.h"

class RandomRoomView : public mge::GamePadWidget {
public:
    RandomRoomView();
    ~RandomRoomView();
private:
    void rebuild();
    void alignRooms();
    void queryMainRoom();
    void makeGraph();
    void makeMiniSpanTree();
    void addSomeEdge();
    void makeCorridor();
private:
    void GenRoom(int room_size, mge::Vector2i const& min_size, mge::Vector2i const& max_size, bool check_overlap = false);
private:
    void onUpdate(float delta) override;
    void draw(SDL_Renderer* renderer) override;
    void onButtonDown(int key) override;
    void addRoom(mge::RectI const& r);
    bool isRoomOverlap(mge::RectI const& r) const;
    int getTileSize() const;

public:
    class Room : public mge::Observer<mge::MaskWidget> {
    public:
        int id;
        mge::RectI r;
        Room():id(0){}
        Room(int id, mge::RectI const& r):id(id), r(r) {}
        void modify();
        void color(SDL_Color const& c);
    };
private:
    int _step;

    mge::Widget* _window;
    mge::Grid<char> _grid;            // 房间网格
    std::vector<Room> _rooms;         // 房间列表
    std::vector<Room*> _mainRoom;     // 主房间列表
    std::map<int, Room*> _passRoom;   // 走廊列表

    struct RoomVertex {
        Room* room;
        float x, y;
    };
    RoomVertex* _roomVertex;                    // 主房间顶点集
    dungeon::EdgeGraph _edgeGraph;              // 三角剖分图
    dungeon::EdgeGraphNoCopy _edgePathGraph;    // 最小生成树路径图

    enum LinkType {
        NONE_LINE = 0,
        VERTICAL_LINE,
        HORIZONTAL_LINE,
        TURNING_LINE,
    };
    std::vector<LinkType> _links;           // 主房间连接类型
    struct Corridor {
        std::vector<mge::Vector2i> tile;
        std::vector<Room*> rooms;
    };
    std::vector<Corridor> _corridors;       // 走廊集，下标和最小生成树路径图对应

    b2World* _world;
    std::map<b2Body*, int> _roomIdx;
    std::vector<std::vector<mge::Vector2f>> _edges;
    std::vector<mge::RectI> _corridorsRects;
};

#endif //SDL2_UI_MENU_H
