//
// Created by baifeng on 2021/9/29.
//

#include "collision.h"

bool isCollision(RectI const& a, RectI const& b) {
    if (a.x >= b.x && a.x >= b.x + b.w) {
        return false;
    } else if (a.x <= b.x && a.x + a.w <= b.x) {
        return false;
    } else if (a.y >= b.y && a.y >= b.y + b.h) {
        return false;
    } else if (a.y <= b.y && a.y + a.h <= b.y) {
        return false;
    }
    return true;
}