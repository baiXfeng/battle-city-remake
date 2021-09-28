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
    Vector<T> operator/(Vector<T> const& vec) const {
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

template<typename T>
class Rect {
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
        return point.x > x and point.x < x + w and point.y > y and point.y < y + h;
    }
    template<typename TargetType>
    Rect<TargetType> to() const {
        return {TargetType(x), TargetType(y)};
    }
};

typedef Rect<int> RectI;
typedef Rect<float> RectF;

#endif //SDL2_UI_TYPES_H
