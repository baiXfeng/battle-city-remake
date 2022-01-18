//
// Created by baifeng on 2021/12/12.
//

#ifndef SDL2_UI_IDMAKER_H
#define SDL2_UI_IDMAKER_H

#include <unordered_map>
#include <string>
#include "macro.h"

mge_begin

class IdMaker final {
public:
    void allclear() {
        _idpool.clear();
    }
    void clear(std::string const& key) {
        _idpool[key] = 0;
    }
    uint32_t gen(std::string const& key) {
        return ++_idpool[key];
    }
protected:
    std::unordered_map<std::string, uint32_t> _idpool;
};

mge_end

#endif //SDL2_UI_IDMAKER_H
