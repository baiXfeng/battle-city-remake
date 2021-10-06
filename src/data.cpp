//
// Created by baifeng on 2021/10/2.
//

#include "data.h"
#include "common/loadres.h"

TileData::TileData():
layer(0),
type(0) {

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