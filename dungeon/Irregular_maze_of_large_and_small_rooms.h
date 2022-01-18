//
// Created by baifeng on 2021/12/17.
//

#ifndef SDL2_UI_IRREGULAR_MAZE_OF_LARGE_AND_SMALL_ROOMS_H
#define SDL2_UI_IRREGULAR_MAZE_OF_LARGE_AND_SMALL_ROOMS_H

#include "common/rect.h"
#include "common/widget.h"
#include "common/grid.h"
#include "box2d/box2d.h"
#include "mini_span_tree.h"
#include "generate.h"

namespace dungeon {

    namespace lasr {

        enum TileType {
            TILE_NONE = 0,
            TILE_STEEL,
        };
        enum RoomType {
            NONE_TYPE = 0,
            MAIN_ROOM,
            PASS_ROOM,
            LINE_CORRIDOR,
            TURNING_CORRIDOR,
        };
        class Room : public mge::Observer<mge::MaskWidget> {
        public:
            int id;
            mge::RectI r;
            RoomType type;
        public:
            Room():id(0), type(NONE_TYPE) {}
            Room(int id, mge::RectI const& r):id(id), r(r) {}
            Room(int id, RoomType type, mge::RectI const& r):id(id), type(type), r(r) {}
            void modify();
            void color(SDL_Color const& c);
            void hide();
            void show();
        };
        struct RoomVertex {
            Room* room;
            float x, y;
        };
        struct Corridor {
            int id; // for edgePathGraph index
            std::vector<Room*> rooms;
        };

        class RoomPathFinding;
        class Data : public mge::Data {
        public:
            Data();
            ~Data();
        public:
            void worldUpdate(float delta);
            bool isWorldSleep() const;

        public:
            int buildRetryCount;
            int tile_size;

            mge::Widget* window;
            mge::Grid<char> grid;            // 房间网格
            std::vector<Room> rooms;         // 房间列表
            std::vector<Room*> mainRoom;     // 主房间列表
            std::vector<Corridor> linkRoom;  // 细长走廊
            std::map<int, Room*> passRoom;   // 走廊房间

            RoomVertex* roomVertex;                    // 主房间顶点集
            dungeon::EdgeGraph edgeGraph;              // 三角剖分图
            dungeon::EdgeGraphNoCopy edgePathGraph;    // 最小生成树路径图(含回环路径)
            dungeon::EdgeGraphNoCopy edgeMiniPathGraph;// 最小生成树(不含环路)
            std::map<int, bool> invalidEdge;           // 无效路径

            b2World* world;
            std::map<b2Body*, int> roomIdx;
            std::vector<std::vector<mge::Vector2f>> edges;

            int start_room;  // 开始房间
            int end_room;    // 最终房间

            std::shared_ptr<RoomPathFinding> path_finding;
        };

        void build_rooms(Context& c);
        void step_world(Context& c);
        void align_rooms(Context& c);
        void make_graph(Context& c);
        void make_mini_span_tree(Context& c);
        void save_start_end_room(Context& c);
        void add_edge(Context& c);
        void make_corridor(Context& c);
        void check_corridor(Context& c);
        void make_center(Context& c);
        void save_map(Context& c);
    }
}

#endif //SDL2_UI_IRREGULAR_MAZE_OF_LARGE_AND_SMALL_ROOMS_H
