//
// Created by baifeng on 2022/1/19.
//

#ifndef SDL2_UI_GRID_H
#define SDL2_UI_GRID_H

#include "macro.h"
#include "vector2.h"

mge_begin

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

#endif //SDL2_UI_GRID_H
