//
// Created by baifeng on 2021/10/2.
//

#include "data.h"
#include "common/loadres.h"
#include "object.h"

TileData::TileData():
layer(0),
type(0) {

}

void TileModel::removeFromScreen() {
    notify_observers(&Widget::removeFromParent);
}

void TankModel::modifyPosition() {
    notify_observers(&Widget::setPosition, position.x, position.y);
}

void TankModel::removeFromScreen() {
    notify_observers(&Widget::removeFromParent);
}

WorldModel::WorldModel():
bounds(0, 0, Tile::MAP_SIZE, Tile::MAP_SIZE),
tiles(0, {0, 0, Tile::MAP_SIZE, Tile::MAP_SIZE}, [](TileModel* tile){
    return tile->bounds;
}), root(nullptr) {

}

namespace res {
    std::string soundName(std::string const& key) {
        return std::string("assets/sounds/") + key + ".ogg";
    }

    std::string imageName(std::string const& key) {
        return std::string("assets/images/") + key + ".png";
    }

    std::string fontName(std::string const& key) {
        return std::string("assets/fonts/") + key + ".ttf";
    }

    std::string levelName(std::string const& key) {
        return getAssetsPath() + "assets/levels/" + key + ".lua";
    }

    std::string levelName(int level) {
        return getAssetsPath() + "assets/levels/level_" + std::to_string(level) + ".lua";
    }
}