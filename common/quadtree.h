//
// Created by baifeng on 2021/9/28.
//

#ifndef SDL2_UI_QUADTREE_H
#define SDL2_UI_QUADTREE_H

#include <memory>
#include <list>
#include <vector>
#include <functional>
#include "types.h"

template<typename T>
class QuadTree {
public:
    typedef std::shared_ptr<QuadTree> Ptr;
    typedef std::shared_ptr<T> SquareOne;
    typedef std::vector<SquareOne> SquareList;
    typedef std::vector<Ptr> QuadTreeList;
    typedef std::function<RectI(SquareOne const&)> RectCatcher;
    enum {
        MAX_LEVELS = 3,
        MAX_OBJECTS = 15,
    };
public:
    QuadTree(int level, RectI const& bounds):_level(level), _bounds(bounds) {
        _nodes.resize(4);
        _catcher = [](SquareOne const& one){
            return RectI{};
        };
    }
    virtual ~QuadTree() {}
public:
    static Ptr New(int level, RectI const& bounds) {
        return Ptr(new QuadTree<T>(level, bounds));
    }
    void retrieve(SquareList& result, RectI const& rect) {
        auto& pRect = rect;
        auto& fSpriteList = result;
        auto indexes = getIndexes(pRect);
        for (auto const& index : indexes) {
            if(index != -1 && _nodes[0] != nullptr) {
                _nodes[index].retrieve(fSpriteList, pRect);
            }
            for (auto& object : _objects) {
                fSpriteList.push_back(object);
            }
        }
    }
    void setCatcher(RectCatcher const& catcher) {
        _catcher = catcher;
    }
    void clear() {
        _objects.clear();
        for (auto& tree : _nodes) {
            if (tree != nullptr) {
                tree->clear();
                tree = nullptr;
            }
        }
    }
    void insert(SquareOne const& sprite) {
        // 从精灵获取区域
        RectI pRect = _catcher(sprite);

        if(_nodes[0] != nullptr) {
            auto indexes = getIndexes(pRect);
            for (auto const& index : indexes) {
                if(index != -1) {
                    _nodes[index]->insert(sprite);
                    return;
                }
            }
        }

        _objects.push_back(sprite);

        if(_objects.size() > MAX_OBJECTS && _level < MAX_LEVELS) {

            if(_nodes[0] == nullptr) {
                split();
            }
            int i = 0;
            while(i < _objects.size()) {
                auto& sqaureOne = _objects[i];
                RectI oRect = _catcher(sqaureOne);
                auto indexes = getIndexes(oRect);
                for (auto const& index : indexes) {
                    if (index != -1) {
                        _nodes[index]->insert(sqaureOne);
                        _objects.erase(_objects.begin()+i);
                    } else {
                        ++i;
                    }
                }
            }
        }
    }
protected:
    void split() {
        int x = (int)_bounds.x;
        int y = (int)_bounds.y;
        int subWidth = (int)(_bounds.w) >> 1;
        int subHeight = (int)(_bounds.h) >> 1;
        _nodes[0] = New(_level + 1, {x + subWidth, y, subWidth, subHeight});
        _nodes[1] = New(_level + 1, {x, y, subWidth, subHeight});
        _nodes[2] = New(_level + 1, {x, y + subHeight, subWidth, subHeight});
        _nodes[3] = New(_level + 1, {x + subWidth, y + subHeight, subWidth, subHeight});
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
protected:
    int _level;
    RectI _bounds;
    SquareList _objects;
    QuadTreeList _nodes;
    RectCatcher _catcher;
};

#endif //SDL2_UI_QUADTREE_H
