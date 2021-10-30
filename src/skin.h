//
// Created by baifeng on 2021/10/13.
//

#ifndef BATTLE_CITY_SKIN_H
#define BATTLE_CITY_SKIN_H

#include <vector>
#include <memory>
#include "data.h"

class Texture;
namespace skin {
    typedef std::shared_ptr<Texture> TexturePtr;
    typedef std::vector<TexturePtr> Textures;
    typedef std::vector<Textures> TexturesArray;

    Textures getShieldSkin();
    Textures getTankAppearSkin();
    TexturesArray getEnemySkin(Tank::Tier tier, bool has_drop);
    TexturesArray getDEnemySkin(Tank::Tier tier);
    TexturesArray getPlayerSkin(Tank::Tier tier, Tank::Controller player);
}

#endif //BATTLE_CITY_SKIN_H
