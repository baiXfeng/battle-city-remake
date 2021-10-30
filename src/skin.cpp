//
// Created by baifeng on 2021/10/13.
//

#include "skin.h"
#include "common/loadres.h"
#include "common/game.h"

namespace skin {

    Textures getShieldSkin() {
        return {
            res::load_texture(_game.renderer(), res::imageName("shield_1")),
            res::load_texture(_game.renderer(), res::imageName("shield_2")),
        };
    }

    Textures getTankAppearSkin() {
        return {
                res::load_texture(_game.renderer(), res::imageName("appear_1")),
                res::load_texture(_game.renderer(), res::imageName("appear_2")),
                res::load_texture(_game.renderer(), res::imageName("appear_3")),
                res::load_texture(_game.renderer(), res::imageName("appear_4")),
        };
    }

    TexturesArray getEnemySkin(Tank::Tier tier, bool has_drop) {
        std::string dir[4] = {"up", "right", "down", "left"};
        std::string name[4] = {"tank_basic_", "tank_fast_", "tank_power_", "tank_armor_"};
        std::string frame[2] = {"_c0_t1", "_c0_t2"};
        frame[1] += has_drop ? "_f" : "";
        TexturesArray ret;
        for (int i = 0; i < 4; ++i) {
            ret.push_back({
                res::load_texture(_game.renderer(), res::imageName(name[tier] + dir[i] + frame[0])),
                res::load_texture(_game.renderer(), res::imageName(name[tier] + dir[i] + frame[1])),
            });
        }
        return ret;
    }

    TexturesArray getDEnemySkin(Tank::Tier tier) {
        std::string dir[4] = {"up", "right", "down", "left"};
        std::string lv[4] = {"_c0", "_c2", "_c2", "_c1"};
        std::string name = "tank_armor_";
        TexturesArray ret;
        for (int i = 0; i < 4; ++i) {
            ret.push_back({
                res::load_texture(_game.renderer(), res::imageName(name + dir[i] + lv[tier] + "_t1")),
                res::load_texture(_game.renderer(), res::imageName(name + dir[i] + lv[tier] + "_t2")),
            });
        }
        return ret;
    }

    TexturesArray getPlayerSkin(Tank::Tier tier, Tank::Controller player) {
        std::string dir[4] = {"up", "right", "down", "left"};
        std::string lv[4] = {"", "_s1", "_s2", "_s3"};
        TexturesArray ret;
        for (int i = 0; i < 4; ++i) {
            std::string name = "tank_player1_";
            ret.push_back({
                res::load_texture(_game.renderer(), res::imageName(name + dir[i] + "_c0_t1" + lv[tier])),
                res::load_texture(_game.renderer(), res::imageName(name + dir[i] + "_c0_t2" + lv[tier])),
            });
        }
        return ret;
    }
}