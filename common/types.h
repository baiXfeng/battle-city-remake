//
// Created by baifeng on 2021/7/12.
//

#ifndef SDL2_UI_TYPES_H
#define SDL2_UI_TYPES_H

template<typename T>
class Vector {
public:
    T x;
    T y;
public:
    Vector<T>():x(0), y(0) {}
    Vector<T>(T const& x, T const& y):x(x), y(y) {}
    Vector<T>& operator=(Vector<T> const& vec) {
        x = vec.x;
        y = vec.y;
        return *this;
    }
    Vector<T> operator+(Vector<T> const& vec) {
        return {x+vec.x, y+vec.y};
    }
    Vector<T>& operator+=(Vector<T> const& vec) {
        x += vec.x;
        y += vec.y;
        return *this;
    }
    Vector<T> operator-(Vector<T> const& vec) {
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
    Vector<T> operator*(Vector<T> const& vec) {
        return {x*vec.x, y*vec.y};
    }
    Vector<T> operator/(Vector<T> const& vec) {
        return {x/vec.x, y/vec.y};
    }
    Vector<T> self_abs() const {
        return {abs(x), abs(y)};
    }
    template<typename TargetType>
    Vector<TargetType> to() const {
        return {TargetType(x), TargetType(y)};
    }
};

typedef Vector<int> Vector2i;
typedef Vector<float> Vector2f;

#endif //SDL2_UI_TYPES_H
