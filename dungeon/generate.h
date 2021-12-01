//
// Created by baifeng on 2021/11/26.
//

#ifndef SDL2_UI_GENERATE_H
#define SDL2_UI_GENERATE_H

#include "common/types.h"

namespace dungeon {

    enum TileType {
        NONE = 0,
        SEA,
        RIVER,
        LAND,
        TREE,
    };

    enum CorridorType {
        ROAD = 0,
        CAVE,
        SHIP_PORT,
        AIR_PORT,
        FOREST,
        GHAT,       // 山路
    };

    class Room;
    class Corridor {
    public:
        Corridor(Room const& r1, Room const& r2);
        virtual ~Corridor() {}
    };

    class Room {
    public:
        virtual ~Room() {}

    protected:
        std::vector<Corridor> _corridors;
    };

    class Builder {
    public:
        virtual void build_room() = 0;
        virtual void link_room() = 0;
        virtual void smooth_corridor() = 0;
        virtual void smooth_room() = 0;
        virtual void fill_corridor() = 0;
        virtual void fill_room() = 0;
    };
}

#endif //SDL2_UI_GENERATE_H
