//
// Created by baifeng on 2021/7/12.
//

#ifndef SDL2_UI_TYPES_H
#define SDL2_UI_TYPES_H

#include "fix.hpp"
#include <functional>
#include <vector>
#include <math.h>
#include <assert.h>

#define mge_begin namespace mge {
#define mge_end }

#if defined(WIN32)
#define GameApp WinMain
#elif defined(__PSP__)
#define GameApp SDL_main
#else
#define GameApp main
#endif

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
    Vector<T> operator/(Vector<T> const& vec) const {
        return {x/vec.x, y/vec.y};
    }
    Vector<T> operator*(T const& v) const {
        return {x*v, y*v};
    }
    Vector<T> operator/(T const& v) const {
        return {x/v, y/v};
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
};

typedef Vector<int> Vector2i;
typedef Vector<float> Vector2f;

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

class Data {
public:
    virtual ~Data() {}
};

template<class T>
class DataPack : public Data {
public:
    template<typename... Args>
    DataPack(Args const&... args):_data(args...) {}
    DataPack(T const& data):_data(data) {}
    T& data() {
        return _data;
    }
    T const& data() const {
        return _data;
    }
private:
    T _data;
};

template<typename T>
T& to(Data* d) {
    auto p = dynamic_cast<T*>(d);
    assert(p && "to type error.");
    return *p;
}

template<typename T>
T* to_p(Data* d) {
    return dynamic_cast<T*>(d);
}

template<class T>
class Grid {
public:
    typedef T Cell;
    typedef std::vector<Cell> CellPool;
    typedef Vector2i Size;
    typedef Vector2i Point;
    typedef int32_t int32;
    typedef uint32_t uint32;
public:
    Grid() {}
    Grid(Grid const& o):_size(o._size), _cells(o._cells) {}
    Grid(Size const& s, CellPool const& arr): _size(s), _cells(arr) {}
    virtual ~Grid() {}
public:
    void resize(uint32 width, uint32 height) {
        _cells.resize(width * height);
        _size.reset(width, height);
    }
    void resize(uint32 width, uint32 height, Cell const& default_value) {
        this->resize(width, height);
        for (int i = 0; i < _cells.size(); ++i) {
            _cells[i] = default_value;
        }
    }
    void resize(Size const& s) {
        this->resize(s.x, s.y);
    }
    void resize(Size const& s, Cell const& default_value) {
        this->resize(s.x, s.y, default_value);
    }
    void clear() {
        _cells.clear();
        _size.reset(0, 0);
    }
    CellPool& cells() {
        return _cells;
    }
    CellPool const& cells() const {
        return _cells;
    }
    Cell& get(uint32 x, uint32 y) {
        assert(not is_out_of_range(x, y) && "Grid::get error.");
        return _cells[y*_size.x+x];
    }
    Cell const& get(uint32 x, uint32 y) const {
        assert(not is_out_of_range(x, y) && "Grid::get error.");
        return _cells[y*_size.x+x];
    }
    Cell& get(Point const& pt) {
        return get(pt.x, pt.y);
    }
    Cell const& get(Point const& pt) const {
        return get(pt.x, pt.y);
    }
    void set(uint32 x, uint32 y, Cell const& c) {
        _cells[y*_size.x+x] = c;
    }
    void set(Point const& pt, Cell const& c) {
        set(pt.x, pt.y, c);
    }
    Size const& size() const {
        return _size;
    }
    inline bool is_out_of_range(int32 x, int32 y) const {
        return x <= -1 or x >= _size.x or y <= -1 or y >= _size.y;
    }
    inline bool is_out_of_range(Point const& pt) const {
        return is_out_of_range(pt.x, pt.y);
    }
    void each(std::function<void(Cell& cell)> const& _call) {
        for (auto& cell : _cells) {
            _call(cell);
        }
    }
    void copy(Grid const& grid, Point const& position) {
        for (int y = 0; y < grid.size().y; ++y) {
            if (position.y + y >= _size.y) {
                break;
            }
            for (int x = 0; x < grid.size().x; ++x) {
                if (position.x + x >= _size.x) {
                    break;
                }
                set(position+Point{x, y}, grid.get(x, y));
            }
        }
    }
protected:
    Size _size;
    CellPool _cells;
};

mge_end

#endif //SDL2_UI_TYPES_H
