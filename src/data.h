//
// Created by baifeng on 2021/10/2.
//

#ifndef SDL2_UI_DATA_H
#define SDL2_UI_DATA_H

#include "common/types.h"
#include <vector>
#include <string>

namespace Tile {

#define IC(name, value) static int const name = value

    IC(SIZE, 40);

    enum Type {
        BASE = 0x100,
        BRICK,
        STEEL,
        WATERS,
        TREES,
        ICE_FLOOR,
        TYPE_END,
    };
}

namespace Tank {
    enum Type {

    };
}

class TileData {
public:
    TileData();
public:
    int layer;
    int type;
};

typedef struct {
    Tile::Type type;
    int x;
    int y;
} AddTile;

typedef struct {
    Tank::Type type;
} AddTank;

typedef std::vector<AddTile> AddTileList;
typedef std::vector<AddTank> AddTankList;

namespace res {
    std::string soundName(std::string const& key);
    std::string imageName(std::string const& key);
    std::string fontName(std::string const& key);
}

#endif //SDL2_UI_DATA_H
