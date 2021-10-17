//
// Created by baifeng on 2021/10/2.
//

#include "data.h"
#include "common/loadres.h"
#include "common/game.h"
#include "object.h"

namespace Tank {
    template <typename T>
    T getValue(lutok3::State& table, std::string const& name) {
        table.getField(-1, name);
        T ret = table.get();
        table.pop();
        return ret;
    }
    Attribute::Attribute() {}
    Attribute::Attribute(lutok3::State& table) {
        health = getValue<int>(table, "health");
        bulletMaxCount = getValue<int>(table, "bulletMaxCount");
        bulletTankDamage = getValue<int>(table, "bulletTankDamage");
        bulletWallDamage = getValue<int>(table, "bulletWallDamage");
        moveSpeed = getValue<float>(table, "moveSpeed");
        bulletRapidFireDelay = getValue<float>(table, "bulletRapidFireDelay");
        bulletSpeed = getValue<float>(table, "bulletSpeed");
    }
    void initSpawns(std::string const& key, std::string const& tableName) {
        // 记录坦克出生点
        auto& state = _game.force_get<lutok3::State>("lua");
        auto& spawns = _game.force_get<Spawns>(key);
        state.getGlobal(tableName);
        state.len(-1);
        int len = state.get();
        state.pop();
        for (int i = 0; i < len; ++i) {
            if (state.getI(-1, i+1) == lutok3::Type::Table) {
                spawns.push_back({
                    getValue<float>(state, "x"),
                    getValue<float>(state, "y"),
                });
            }
            state.pop();
        }
        state.pop();
    }
    void loadTankSpawns() {
        initSpawns("player_spawns", "PLAYER_DEFAULT_SPAWN_POSITIONS");
        initSpawns("enemy_spawns", "ENEMY_DEFAULT_SPAWN_POSITIONS");
    }
    Spawns const& getSpawns(Party group) {
        std::string key[2] = {"player_spawns", "enemy_spawns"};
        return _game.get<Spawns>(key[group]);
    }
    void loadAttributes(Attributes& ret, lutok3::State& state, std::string const& key) {
        if (state.getGlobal(key) == lutok3::Type::Table) {
            state.len(-1);
            int len = state.get();
            state.pop();
            for (int i = 0; i < len; ++i) {
                if (state.getI(-1, i+1) == lutok3::Type::Table) {
                    ret.push_back(Attribute(state));
                }
                state.pop();
            }
        }
        state.pop();
    }
    void loadAttributes() {
        auto& state = _game.force_get<lutok3::State>("lua");
        auto& config = _game.force_get<Config>("tank:config");
        std::string name[2] = {
                "PLAYER_TANK_ATTRIBUTES",
                "ENEMY_TANK_ATTRIBUTES",
        };
        config.clear();
        config.resize(2);
        for (int i = 0; i < 2; ++i) {
            auto& attrs = config[i];
            loadAttributes(attrs, state, name[i]);
            assert(attrs.size() == Tier::D+1 and "tank::loadAttributes error.");
        }
    }
    Attribute const& getAttribute(Party group, Tier tier) {
        return _game.get<Config>("tank:config")[group][tier];
    }
    int getDefaultLifeMax() {
        auto& state = _game.force_get<lutok3::State>("lua");
        int ret = 0;
        if (state.getGlobal("DEFAULT_LIFE_MAX") == lutok3::Type::Number) {
            ret = state.get();
        }
        state.pop();
        return ret;
    }
}

TankModel::TankModel():
id(0),
hp(0),
fire(false),
shield(false),
has_drop(false),
party(Tank::PLAYER),
tier(Tank::A),
dir(Tank::MAX),
controller(Tank::AI) {

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

void TankModel::createBullet() {
    notify_observers(&TankView::onFire);
}

void TankModel::createExplosion() {
    notify_observers(&TankView::explosion);
}

void TankModel::createScore() {
    notify_observers(&TankView::show_score);
}

void BulletModel::modifyPosition() {
    notify_observers(&Widget::setPosition, position.x, position.y);
}

void BulletModel::removeFromScreen() {
    notify_observers(&Widget::removeFromParent);
}

void BulletModel::playExplosion() {
    notify_observers(&BulletView::play_explosion);
}

void PropModel::createScore() {
    notify_observers(&PropView::show_score);
}

void PropModel::removeFromScreen() {
    notify_observers(&Widget::removeFromParent);
}

WorldModel::WorldModel():
bounds(0, 0, Tile::MAP_SIZE, Tile::MAP_SIZE),
tiles(0, {0, 0, Tile::MAP_SIZE, Tile::MAP_SIZE}, [](TileModel* tile){
    return tile->bounds;
}) {}

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

    std::string scriptName(std::string const& key) {
        return getAssetsPath() + "assets/scripts/" + key + ".lua";
    }

    std::string powerupName(Tank::PowerUp e) {
        assert(e != Tank::POWER_MAX and "res::powerupName error.");
        std::string name[Tank::POWER_MAX] = {
                "powerup_grenade",
                "powerup_helmet",
                "powerup_shovel",
                "powerup_star",
                "powerup_tank",
                "powerup_timer",
        };
        return imageName(name[e]);
    }

    std::string assetsName(std::string const& fileName) {
        return getAssetsPath() + "assets/" + fileName;
    }
}