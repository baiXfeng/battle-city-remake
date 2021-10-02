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

template<typename T>
class QuadTree {
public:
    typedef std::shared_ptr<QuadTree> Ptr;
    typedef std::shared_ptr<T> Square;
    typedef std::list<Square> SquareList;
    typedef std::vector<Ptr> QuadTreeList;
    typedef std::function<RectI(Square const&)> RectCatcher;
    typedef std::unordered_map<void*, RectI> RectCache;
    enum {
        MAX_LEVELS = 4,
        MAX_OBJECTS = 15,
    };
public:
    QuadTree(int level, RectI const& bounds, RectCatcher const& catcher):_level(level), _bounds(bounds), _catcher(catcher) {}
    virtual ~QuadTree() {}
public:
    static Ptr New(int level, RectI const& bounds, RectCatcher const& catcher) {
        return Ptr(new QuadTree<T>(level, bounds, catcher));
    }
    void retrieve(SquareList& result, RectI const& rect) {
        auto& pRect = rect;
        auto& fSpriteList = result;
        auto indexes = getIndexes(pRect);
        for (auto const& index : indexes) {
            if(index != -1 && _nodes.size()) {
                _nodes[index]->retrieve(fSpriteList, pRect);
            }
            for (auto& object : _objects) {
                fSpriteList.push_back(object);
            }
        }
    }
    void clear() {
        _rects.clear();
        _objects.clear();
        _nodes.clear();
    }
    void insert(Square const& sprite, bool save_rect = true) {

        RectI pRect = _catcher(sprite);
        if (save_rect) {
            _rects[sprite.get()] = pRect;
        }

        if(_nodes.size()) {
            auto indexes = getIndexes(pRect);
            for (auto const& index : indexes) {
                if(index != -1) {
                    _nodes[index]->insert(sprite, false);
                    return;
                }
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
                for (auto const& index : indexes) {
                    if (index != -1) {
                        _nodes[index]->insert(sqaureOne, false);
                        _objects.erase(iter++);
                    } else {
                        ++iter;
                    }
                    break;
                }
            }
        }
    }
    void remove(Square const& sprite) {
        auto iter = _rects.find(sprite.get());
        if (iter == _rects.end()) {
            return;
        }
        auto& rect = iter->second;
        auto tree = find(rect);
        tree->erase(sprite);
        _rects.erase(iter);
    }
protected:
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
    QuadTree* find(RectI const& rect) {
        if (_nodes.size()) {
            auto indexes = getIndexes(rect);
            for (auto const& index : indexes) {
                if(index != -1) {
                    return _nodes[index]->find(rect);
                }
            }
        }
        return this;
    }
protected:
    int _level;
    RectI _bounds;
    SquareList _objects;
    QuadTreeList _nodes;
    RectCatcher _catcher;
    RectCache _rects;
};

#endif //SDL2_UI_QUADTREE_H
