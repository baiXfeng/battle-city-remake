//
// Created by baifeng on 2022/1/19.
//

#ifndef SDL2_UI_RECT_H
#define SDL2_UI_RECT_H

#include "macro.h"
#include "vector2.h"

mge_begin

    template<typename T>
    class Rect {
    public:
        typedef T Type;
    public:
        T x;
        T y;
        T w;
        T h;
    public:
        virtual ~Rect<T>() {}
        Rect<T>():x(0), y(0), w(0), h(0) {}
        Rect<T>(T const& x, T const& y, T const& w, T const& h):x(x), y(y), w(w), h(h) {}
        Rect<T>(Rect<T> const& r):x(r.x), y(r.y), w(r.w), h(r.h) {}
        Rect<T>& operator=(Rect<T> const& r) {
            x = r.x;
            y = r.y;
            w = r.w;
            h = r.h;
            return *this;
        }
        inline bool contain(Vector<T> const& point) const {
            return horizontal_contain(point) and vertical_contain(point);
        }
        inline bool horizontal_contain(Vector<T> const& point) const {
            return point.x > x and point.x < x + w;
        }
        inline bool vertical_contain(Vector<T> const& point) const {
            return point.y > y and point.y < y + h;
        }
        template<typename TargetType>
        Rect<TargetType> to() const {
            return {TargetType(x), TargetType(y), TargetType(w), TargetType(h)};
        }
        Vector<T> center() const {
            return {
                    x + (w / 2),
                    y + (h / 2),
            };
        }
    };

    typedef Rect<int> RectI;
    typedef Rect<float> RectF;

mge_end

#endif //SDL2_UI_RECT_H
