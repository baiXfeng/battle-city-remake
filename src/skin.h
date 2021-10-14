//
// Created by baifeng on 2021/10/13.
//

#ifndef SDL2_UI_SKIN_H
#define SDL2_UI_SKIN_H

#include <vector>
#include <memory>
#include "data.h"

class Texture;
namespace skin {
    typedef std::shared_ptr<Texture> TexturePtr;
    typedef std::vector<TexturePtr> Textures;
    typedef std::vector<Textures> TexturesArray;

    Textures getShieldSkin();
    TexturesArray getEnemySkin(Tank::Tier tier, bool has_drop);
    TexturesArray getTopEnemySkin();
    TexturesArray getPlayerSkin(Tank::Tier tier, Tank::Controller player);
}

#endif //SDL2_UI_SKIN_H
