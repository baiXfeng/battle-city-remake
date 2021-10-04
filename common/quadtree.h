//
// Created by baifeng on 2021/9/28.
//

#ifndef SDL2_UI_QUADTREE_H
#define SDL2_UI_QUADTREE_H

#include <memory>
#include <list>
#include <vector>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include "types.h"
#include "render.h"

template<typename T>
class QuadTree {
public:
    typedef T Square;
    typedef std::shared_ptr<QuadTree> Ptr;
    typedef std::list<Square> SquareList;
    typedef std::vector<Ptr> QuadTreeList;
    typedef std::function<RectI(Square const&)> RectCatcher;
    typedef std::function<void*(T const&)> Pointer;
    enum {
        MAX_LEVELS = 4,
        MAX_OBJECTS = 10,
    };
public:
    QuadTree(int level, RectI const& bounds, RectCatcher const& catcher):_level(level), _bounds(bounds), _catcher(catcher) {}
    virtual ~QuadTree() {}
public:
    static Ptr New(int level, RectI const& bounds, RectCatcher const& catcher) {
        return Ptr(new QuadTree<T>(level, bounds, catcher));
    }
    void unique(SquareList& result, Pointer const& get) {
        std::map<void*, bool> flags;
        for (auto iter = result.begin(); iter != result.end();) {
            auto key = get(*iter);
            auto& value = flags[key];
            if (value) {
                result.erase(iter++);
            } else {
                value = true;
                ++iter;
            }
        }
    }
    void retrieve(SquareList& result, RectI const& rect) {
        auto indexes = getIndexes(rect);
        if (_nodes.size() and indexes.front() != -1) {
            for (auto const& index : indexes) {
                _nodes[index]->retrieve(result, rect);
            }
        }
        for (auto& object : _objects) {
            result.push_back(object);
        }
    }
    void clear() {
        _objects.clear();
        _nodes.clear();
    }
    void insert(Square const& sprite) {
        insert_square(sprite, _catcher(sprite));
    }
    void remove(Square const& sprite) {
        remove_square(sprite, _catcher(sprite));
    }
    QuadTreeList const& nodes() const {
        return _nodes;
    }
    RectI const& bound() const {
        return _bounds;
    }
protected:
    void remove_square(Square const& sprite, RectI const& rect) {
        auto indexes = getIndexes(rect);
        if (_nodes.empty() or indexes.front() == -1) {
            this->erase(sprite);
        } else if (_nodes.size()) {
            for (auto const& index : indexes) {
                _nodes[index]->remove_square(sprite, rect);
            }
        }
    }
    void insert_square(Square const& sprite, RectI const& rect) {
        if(_nodes.size()) {
            auto indexes = getIndexes(rect);
            if (indexes.front() != -1) {
                for (auto const& index : indexes) {
                    _nodes[index]->insert_square(sprite, rect);
                }
                return;
            }
        }
        _objects.push_back(sprite);

        if(_objects.size() > MAX_OBJECTS && _level < MAX_LEVELS) {
            if(_nodes.empty()) {
                split();
            }
            for (auto iter = _objects.begin(); iter != _objects.end();) {
                auto& sqaureOne = *iter;
                RectI oRect = _catcher(sqaureOne);
                auto indexes = getIndexes(oRect);
                if (indexes.front() == -1) {
                    ++iter;
                } else {
                    for (auto const& index : indexes) {
                        _nodes[index]->insert_square(sqaureOne, rect);
                    }
                    _objects.erase(iter++);
                }
            }
        }
    }
    void split() {
        int x = (int)_bounds.x;
        int y = (int)_bounds.y;
        int subWidth = (int)(_bounds.w) >> 1;
        int subHeight = (int)(_bounds.h) >> 1;
        _nodes.resize(4);
        _nodes[0] = New(_level + 1, {x + subWidth, y, subWidth, subHeight}, _catcher);
        _nodes[1] = New(_level + 1, {x, y, subWidth, subHeight}, _catcher);
        _nodes[2] = New(_level + 1, {x, y + subHeight, subWidth, subHeight}, _catcher);
        _nodes[3] = New(_level + 1, {x + subWidth, y + subHeight, subWidth, subHeight}, _catcher);
    }
    std::vector<int> getIndexes(RectI const& rect) {
        auto& pRect = rect;
        std::vector<int> indexes;

        double verticalMidpoint = _bounds.x + (_bounds.w >> 1);
        double horizontalMidpoint = _bounds.y + (_bounds.h >> 1);

        bool topQuadrant = pRect.y >= horizontalMidpoint;
        bool bottomQuadrant = (pRect.y - pRect.h) <= horizontalMidpoint;
        bool topAndBottomQuadrant = pRect.y + pRect.h + 1 >= horizontalMidpoint && pRect.y + 1 <= horizontalMidpoint;

        if(topAndBottomQuadrant) {
            topQuadrant = false;
            bottomQuadrant = false;
        }

        if(pRect.x + pRect.w + 1 >= verticalMidpoint && pRect.x -1 <= verticalMidpoint) {
            // Check if object is in left and right quad
            if(topQuadrant) {
                indexes.push_back(2);
                indexes.push_back(3);
            } else if(bottomQuadrant) {
                indexes.push_back(0);
                indexes.push_back(1);
            } else if(topAndBottomQuadrant) {
                indexes.push_back(0);
                indexes.push_back(1);
                indexes.push_back(2);
                indexes.push_back(3);
            }
        } else if(pRect.x + 1 >= verticalMidpoint) {
            // Check if object is in just right quad
            if(topQuadrant) {
                indexes.push_back(3);
            } else if(bottomQuadrant) {
                indexes.push_back(0);
            } else if(topAndBottomQuadrant) {
                indexes.push_back(3);
                indexes.push_back(0);
            }
        } else if(pRect.x - pRect.w <= verticalMidpoint) {
            // Check if object is in just left quad
            if(topQuadrant) {
                indexes.push_back(2);
            } else if(bottomQuadrant) {
                indexes.push_back(1);
            } else if(topAndBottomQuadrant) {
                indexes.push_back(2);
                indexes.push_back(1);
            }
        } else {
            indexes.push_back(-1);
        }
        return indexes;
    }
    void erase(Square const& sprite) {
        auto iter = std::find(_objects.begin(), _objects.end(), sprite);
        if (iter != _objects.end()) {
            _objects.erase(iter);
        }
    }
protected:
    int _level;
    RectI _bounds;
    SquareList _objects;
    QuadTreeList _nodes;
    RectCatcher _catcher;
};

template<class T>
class DebugQuadTree : public QuadTree<T> {
public:
    typedef std::shared_ptr<DebugQuadTree<T>> Ptr;
    typedef QuadTree<T> QuadTreeT;
    typedef typename QuadTree<T>::RectCatcher RectCatcher;
public:
    DebugQuadTree(int level, RectI const& bounds, RectCatcher const& catcher):QuadTreeT(level, bounds, catcher) {}
    void draw(SDL_Renderer* renderer, Vector2i const& position = {0, 0}) {
        draw(renderer, *this, position);
    }
private:
    void draw(SDL_Renderer* renderer, QuadTreeT const& quadtree, Vector2i const& position) {
        RenderDrawRect rect;
        rect.setColor({255, 255, 255, 255});
        rect.setSize(quadtree.bound().w-1, quadtree.bound().h-1);
        rect.draw(renderer, position + Vector2i{quadtree.bound().x, quadtree.bound().y});
        for (auto& node : quadtree.nodes()) {
            draw(renderer, *node.get(), position);
        }
    }
};

#endif //SDL2_UI_QUADTREE_H
