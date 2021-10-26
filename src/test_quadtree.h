//
// Created by baifeng on 2021/10/3.
//

#ifndef BATTLE_CITY_TEST_QUADTREE_H
#define BATTLE_CITY_TEST_QUADTREE_H

class Object : public RenderFillRect {
public:
    Object():_visible(true), _blink(false), _timer(0.0f), _timer_max(0.15f) {
        setColor({255, 0, 0, 255});
        setSize(40, 40);
    }
    void setMove(Vector2f const& move) {
        _move = move;
    }
    void setMoveX(float v) {
        _move.x = v;
    }
    void setMoveY(float v) {
        _move.y = v;
    }
    void setPosition(Vector2f const& position) {
        _position = position;
    }
    Vector2f const& position() const {
        return _position;
    }
    void setBlink(bool blink) {
        _blink = blink;
        if (!_blink) {
            _visible = true;
        }
    }
    bool blink() const {
        return _blink;
    }
    void update(float delta) {
        _position += _move * delta;
        if (_position.x <= 0) {
            _position.x = 0.0f;
            _move.x = 0.0f;
        } else if (_position.x >= 960-40) {
            _position.x = 960 - 40;
            _move.x = 0.0f;
        }
        if (_position.y <= 0) {
            _position.y = 0.0f;
            _move.y = 0.0f;
        } else if (_position.y >= 544-40) {
            _position.y = 544 - 40;
            _move.y = 0.0f;
        }
        if (_blink) {
            if ((_timer += delta) >= _timer_max) {
                _timer -= _timer_max;
                _visible = !_visible;
            }
        }
    }
    void draw(SDL_Renderer* renderer, Vector2i const& position = {0, 0}) override {
        if (!_visible) {
            return;
        }
        RenderFillRect::draw(renderer, position + _position.to<int>());
    }
    RectI rect() const {
        return RectI{
                int(_position.x),
                int(_position.y),
                size().x,
                size().y,
        };
    }
private:
    bool _blink;
    bool _visible;
    float _timer, _timer_max;
    Vector2f _position;
    Vector2f _move;
};

RectI __getRect(std::shared_ptr<Object> const& obj) {
    return obj->rect();
}

class MyScene : public GamePadWidget {
private:
    typedef std::shared_ptr<Object> ObjectPtr;
    typedef DebugQuadTree<ObjectPtr> DebugQuadTreeT;
    typedef DebugQuadTreeT::Ptr _QuadTreePtr;
    std::list<ObjectPtr> _objects;
    ObjectPtr _player;
    _QuadTreePtr _quadtree;
    DebugQuadTreeT::SquareList _prev_list;

public:
    MyScene():_player(std::make_shared<Object>()), _quadtree(
            std::make_shared<DebugQuadTreeT>(0, RectI{0, 0, 960, 544}, __getRect)) {
        _objects.push_back(_player);
        this->reset();
    }
    void reset() {
        _prev_list.clear();
        while (_objects.size() != 1) {
            _objects.pop_back();
        }
        _quadtree->clear();
        for (int i = 0; i < 50; ++i) {
            auto obj = std::make_shared<Object>();
            obj->setColor({0, 255, 0, 255});
            obj->setPosition({rand() % 960 - 40.0f, rand() % 544 - 40.0f});
            _objects.push_back(obj);
            _quadtree->insert(obj);
        }
    }
    void update(float dt) override {
        for (auto& obj : _objects) {
            obj->update(dt);
        }
        std::map<Object*, bool> flags;
        for (auto& obj : _prev_list) {
            flags[obj.get()] = false;
        }
        DebugQuadTreeT::SquareList ret;
        _quadtree->retrieve(ret, _player->rect());
        _prev_list = ret;
        for (auto& obj : _prev_list) {
            flags[obj.get()] = true;
        }
        for (auto& iter : flags) {
            iter.first->setBlink(iter.second);
        }
    }
    void draw(SDL_Renderer* renderer) override {
        for (auto& obj : _objects) {
            obj->draw(renderer);
        }
        _quadtree->draw(renderer);
    }
    void onButtonDown(int key) override {
        if (key == KeyCode::UP) {
            _player->setMove({0, -300});
        } else if (key == KeyCode::DOWN) {
            _player->setMove({0, 300});
        } else if (key == KeyCode::LEFT) {
            _player->setMove({-300, 0});
        } else if (key == KeyCode::RIGHT) {
            _player->setMove({300, 0});
        } else if (key == KeyCode::START) {
            this->reset();
        } else if (key == KeyCode::SELECT) {
            DebugQuadTreeT::SquareList ret;
            _quadtree->retrieve(ret, _player->rect());
            /*_quadtree->unique(ret, [](DebugQuadTree::Square const& obj) {
                return obj.get();
            });*/
            printf("碰撞数量: %d\n", int(ret.size()));
        } else if (key == KeyCode::X) {
            DebugQuadTreeT::SquareList ret;
            _quadtree->retrieve(ret, _player->rect());
            _quadtree->unique(ret, [](DebugQuadTreeT::Square const& obj) {
                return obj.get();
            });
            printf("碰撞数量: %d\n", int(ret.size()));
        } else if (key == KeyCode::Y) {
            _quadtree->insert(_player);
            _quadtree->remove(_player);
        }
    }
    void onButtonUp(int key) override {
        if (key == KeyCode::UP) {
            _player->setMoveY(0);
        } else if (key == KeyCode::DOWN) {
            _player->setMoveY(0);
        } else if (key == KeyCode::LEFT) {
            _player->setMoveX(0);
        } else if (key == KeyCode::RIGHT) {
            _player->setMoveX(0);
        }
    }
};

#endif //BATTLE_CITY_TEST_QUADTREE_H
