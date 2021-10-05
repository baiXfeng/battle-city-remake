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
    int type = state.get(1);
    list.push_back({Tank::Type(type)});
    return 0;
}

void registerLuaFunctions(lutok3::State& state) {

    std::string tileName[6] = {"base", "brick", "steel", "waters", "trees", "ice_floors"};
    for (int i = Tile::BASE; i < Tile::TYPE_END; ++i) {
        int index = i - Tile::BASE;
        registerValue(state, "tile", tileName[index], i);
    }
    registerValue(state, "tile", "size", Tile::SIZE);

    //std::string tankName[] = {};

    state.pushFunction(&add_tile);
    state.setGlobal("add_tile");

    state.pushFunction(&add_tank);
    state.setGlobal("add_tank");
}
