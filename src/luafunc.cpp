//
// Created by baifeng on 2021/10/5.
//

#include "luafunc.h"
#include "data.h"
#include "common/game.h"

typedef int(*LuaCFunc)(lutok3::State& state);
static void registerCFunction(lutok3::State& state, std::string const& tableName, std::string const& funcName, LuaCFunc func) {
    auto type = state.getGlobal(tableName);
    if (type == lutok3::Type::Nil) {
        state.pop();
        state.newTable();
        state.setGlobal(tableName);
        state.getGlobal(tableName);
    }
    auto index = state.getTop();
    state.push(funcName.c_str());
    state.pushFunction(func);
    state.setTable(index);
    state.pop();
}

template <typename T>
static void registerValue(lutok3::State& state, std::string const& tableName, std::string const& keyName, T value) {
    auto type = state.getGlobal(tableName);
    if (type == lutok3::Type::Nil) {
        state.pop();
        state.newTable();
        state.setGlobal(tableName);
        state.getGlobal(tableName);
    }
    auto index = state.getTop();
    state.push(keyName.c_str());
    state.push<T>(value);
    state.setTable(index);
    state.pop();
}

static std::map<std::string, bool> _importSets;

int import(lutok3::State& state) {
    if (state.getTop() == 0) {
        return 0;
    }
    std::string file = state.get();
    if (_importSets[file] == true) {
        return 0;
    }
    _importSets[file] = true;
    state.doFile(res::assetsName(file));
    return 0;
}

int add_tile(lutok3::State& state) {
    auto& list = _game.force_get<AddTileList>("add_tile_list");
    int type = state.get(1);
    float x = state.get(2);
    float y = state.get(3);
    list.push_back({Tile::Type(type), int(x), int(y)});
    return 0;
}

int add_tank(lutok3::State& state) {
    auto& list = _game.force_get<AddTankList>("add_tank_list");
    int top = state.getTop();
    if (top == 0) {
        return 0;
    }
    bool has_drop = false;
    int tier = state.get(1);
    if (state.type(2) == lutok3::Type::Boolean) {
        has_drop = state.get(2);
    }
    list.push_back({
        has_drop, Tank::Tier(tier),
    });
    return 0;
}

void registerLuaFunctions(lutok3::State& state) {

    std::string const tileName[6] = {"base", "brick", "steel", "waters", "trees", "ice_floors"};
    for (int i = Tile::BASE; i < Tile::TYPE_END; ++i) {
        int index = i - Tile::BASE;
        registerValue(state, "tile", tileName[index], i);
    }
    registerValue(state, "tile", "size", Tile::SIZE);

    std::string const tankName[Tank::TIER_MAX] = {"a", "b", "c", "d"};
    Tank::Tier const tier[Tank::TIER_MAX] = {
            Tank::A,
            Tank::B,
            Tank::C,
            Tank::D,
    };
    for (int i = 0; i < Tank::TIER_MAX; ++i) {
        registerValue(state, "tank", tankName[i], (int)tier[i]);
    }

    state.pushFunction(&import);
    state.setGlobal("import");

    state.pushFunction(&add_tile);
    state.setGlobal("add_tile");

    state.pushFunction(&add_tank);
    state.setGlobal("add_tank");
}
