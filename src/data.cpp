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
        char name[128] = {0};
        sprintf(name, "%sassets/sounds/%s.ogg", res::getAssetsPath().c_str(), key.c_str());
        return name;
    }

    std::string imageName(std::string const& key) {
        char name[128] = {0};
        sprintf(name, "%sassets/images/%s.ogg", res::getAssetsPath().c_str(), key.c_str());
        return name;
    }

    std::string fontName(std::string const& key) {
        char name[128] = {0};
        sprintf(name, "%sassets/fonts/%s.ogg", res::getAssetsPath().c_str(), key.c_str());
        return name;
    }

    std::string levelName(std::string const& key) {
        char name[128] = {0};
        sprintf(name, "%sassets/levels/%s.lua", res::getAssetsPath().c_str(), key.c_str());
        return name;
    }
}