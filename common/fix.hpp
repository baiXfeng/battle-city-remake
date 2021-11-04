//
// Created by baifeng on 2021/11/3.
//

#ifndef SDL2_UI_FIX_HPP
#define SDL2_UI_FIX_HPP

#include <string>
#include <stdio.h>

#if defined(__PSP__)

namespace std {
    template<typename T> std::string to_string(T const& value) {
        char temp[80] = {0};
        sprintf(temp, "%d", (int)value);
        return temp;
    }
    template<typename T> std::string to_string(double const& value) {
        char temp[80] = {0};
        sprintf(temp, "%lf", value);
        return temp;
    }
    template<typename T> std::string to_string(float const& value) {
        char temp[80] = {0};
        sprintf(temp, "%f", value);
        return temp;
    }
}

#endif

#endif //SDL2_UI_FIX_HPP
