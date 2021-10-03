//
// Created by baifeng on 2021/10/2.
//

#ifndef SDL2_UI_DATA_H
#define SDL2_UI_DATA_H

#include "common/types.h"

namespace Tile {
    static int const SIZE = 40;
}

class TileData {
public:
    TileData();
public:
    int layer;
    int type;
};

#endif //SDL2_UI_DATA_H
