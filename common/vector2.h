//
// Created by baifeng on 2022/1/19.
//

#ifndef SDL2_UI_VECTOR2_H
#define SDL2_UI_VECTOR2_H

#include "macro.h"
#include "math.h"

mge_begin

    template<typename T>
    class Vector {
    public:
        typedef T Type;
    public:
        Type x;
        Type y;
    public:
        virtual ~Vector<T>() {}
        Vector<T>():x(0), y(0) {}
        Vector<T>(T const& x, T const& y):x(x), y(y) {}
        Vector<T>(Vector<T> const& vec):x(vec.x), y(vec.y) {}
        Vector<T>& operator=(Vector<T> const& vec) {
            x = vec.x;
            y = vec.y;
            return *this;
        }
        Vector<T> operator+(Vector<T> const& vec) const {
            return {x+vec.x, y+vec.y};
        }
        Vector<T>& operator+=(Vector<T> const& vec) {
            x += vec.x;
            y += vec.y;
            return *this;
        }
        Vector<T> operator-(Vector<T> const& vec) const {
            return {x-vec.x, y-vec.y};
        }
        Vector<T>& operator-=(Vector<T> const& vec) {
            x -= vec.x;
            y -= vec.y;
            return *this;
        }
        bool operator==(Vector<T> const& vec) const {
            return x == vec.x and y == vec.y;
        }
        Vector<T> operator*(Vector<T> const& vec) const {
            return {x*vec.x, y*vec.y};
        }
        Vector<T>& operator*=(Vector<T> const& vec) {
            x *= vec.x;
            y *= vec.y;
            return *this;
        }
        Vector<T> operator/(Vector<T> const& vec) const {
            return {x/vec.x, y/vec.y};
        }
        Vector<T>& operator/=(Vector<T> const& vec) {
            x /= vec.x;
            y /= vec.y;
            return *this;
        }
        Vector<T> operator*(T const& v) const {
            return {x*v, y*v};
        }
        Vector<T>& operator*=(T const& v) {
            x *= v;
            y *= v;
            return *this;
        }
        Vector<T> operator/(T const& v) const {
            return {x/v, y/v};
        }
        Vector<T>& operator/=(T const& v) const {
            x /= v;
            y /= v;
            return *this;
        }
        Vector<T> self_abs() const {
            return {abs(x), abs(y)};
        }
        void reset(T const& x, T const& y) {
            this->x = x;
            this->y = y;
        }
        void reset(Vector<T> const& vec) {
            *this = vec;
        }
        void reset() {
            reset((T)0, (T)0);
        }
        template<typename TargetType>
        Vector<TargetType> to() const {
            return {TargetType(x), TargetType(y)};
        }
        double distance(Vector<T> const& vec) const {
            Type temp_x = x - vec.x;
            Type temp_y = y - vec.y;
            return sqrt(temp_x*temp_x+temp_y*temp_y);
        }
        void normalize() {
            auto l = x * x + y * y;
            if (l != 0) {
                l = sqrt(l);
                x /= l;
                y /= l;
            }
        }
        Vector<float> normalized() {
            Vector<T> ret{x, y};
            ret.normalize();
            return ret;
        }
    };

    typedef Vector<int> Vector2i;
    typedef Vector<float> Vector2f;

mge_end

#endif //SDL2_UI_VECTOR2_H
