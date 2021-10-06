//
// Created by baifeng on 2021/10/7.
//

#include "battle.h"
#include "data.h"
#include "view.h"
#include "lutok3.h"
#include "common/action.h"
#include "common/game.h"
#include "common/collision.h"
#include "common/loadres.h"
#include "common/audio.h"
#include "const.h"

//=====================================================================================

static RectI tileRectCatcher(QuadTree<Widget::Ptr>::Square const& square) {
    return RectI{
            int(square->position().x - square->size().x * square->anchor().x),
            int(square->position().y - square->size().y * square->anchor().y),
            int(square->size().x),
            int(square->size().y),
    };
}

BattleFieldView::BattleFieldView():
        _root(nullptr),
        _player(nullptr),
        _pause(false) {

    auto old_size = this->size();
    this->setSize(Tile::MAP_SIZE, Tile::MAP_SIZE);

    _quadtree = DebugQuadTreeT::Ptr(new DebugQuadTreeT(0, {0, 0, int(size().x), int(size().y)}, tileRectCatcher));

    Ptr widget;

    {
        auto view = Ptr(new MaskWidget({0, 0, 0, 255}));
        view->setSize(size());
        addChild(view);
    }

    {
        auto view = New<WindowWidget>();
        view->setSize(size());
        addChild(view);
        _root = view.get();
    }

    onLoadLevel();
    sortElements();
}

void BattleFieldView::onLoadLevel() {
    auto& tile_list = _game.force_get<AddTileList>("add_tile_list");
    tile_list.clear();

    auto level = _game.get<int>("level");
    auto file = res::levelName(level);

    auto& state = _game.get<lutok3::State>("lua");
    state.doFile(file);

    TileBuilder::TileArray array;
    TileBuilder builder;
    builder.gen(array, tile_list);

    for (auto& widget : array) {
        addElement(widget);
    }
}

void BattleFieldView::onUpdate(float delta) {
    //procTankControl();
}

void BattleFieldView::draw(SDL_Renderer* renderer) {
    if (!_visible) {
        return;
    }
    for (auto& child : _children) {
        child->draw(renderer);
    }
    //onDraw(renderer);
}

void BattleFieldView::onDraw(SDL_Renderer* renderer) {
    _quadtree->draw(renderer, _global_position.to<int>());
}

void BattleFieldView::onTankUpdateQuadTree(Widget::Ptr const& tank) {
    _quadtree->remove(tank);
    _quadtree->insert(tank);
}

void BattleFieldView::onTankMoveCollision(TankView* tank) {
    return;
    std::map<Widget*, bool> flags;
    if (_checklist.size()) {
        for (auto& widget : _checklist) {
            flags[widget.get()] = false;
        }
    }
    auto position = tank->position() - tank->size() * tank->anchor();
    RectI rect{
            int(position.x),
            int(position.y),
            int(tank->size().x),
            int(tank->size().y),
    };
    WidgetQuadTree::SquareList result;
    _quadtree->retrieve(result, rect);
    _quadtree->unique(result, [](WidgetQuadTree::Square const& obj){
        return obj.get();
    });
    _checklist = result;
    printf("对象: %d\n", (int)result.size());

    for (auto& widget : _checklist) {
        flags[widget.get()] = true;
    }

    for (auto& iter : flags) {
        if (iter.first == tank) {
            continue;
        }
        if (iter.second) {
            if (!iter.first->hasAction("tile-blink")) {
                auto widget = iter.first;
                auto blink = Action::Ptr(new Blink(widget, 5, 1.0f));
                auto repeat = Action::New<Repeat>(blink);
                repeat->setName("tile-blink");
                widget->runAction(repeat);
            }
        } else {
            iter.first->stopAction("tile-blink");
            iter.first->setVisible(true);
        }
    }

    for (auto& widget : result) {
        if (widget.get() == tank) {
            continue;
        }
        RectI r{
                int(widget->position().x),
                int(widget->position().y),
                int(widget->size().x),
                int(widget->size().y),
        };
        if (isCollision(rect, r)) {
            _quadtree->remove(widget);
            widget->removeFromParent();

            auto iter = std::find(_checklist.begin(), _checklist.end(), widget);
            if (iter != _checklist.end()) {
                _checklist.erase(iter);
            }
        }
    }
}

void BattleFieldView::onButtonDown(int key) {
    if (key >= KeyCode::UP and key <= KeyCode::RIGHT) {
        add_key(key);
    } else if (key == KeyCode::START) {
        //gameOver();
        pause(!_pause);
    } else if (key == KeyCode::SELECT) {
        //this->onTankUpdateQuadTree(_player->ptr());
        gameOver();
    } else if (key == KeyCode::L1) {
        if (!_pause) {
            pause(true);
        }
        auto widget = Widget::New<CheatView>([&]{
            pause(false);
        });
        _game.screen().scene_back()->addChild(widget);
    }
}

void BattleFieldView::onButtonUp(int key) {
    if (key >= KeyCode::UP and key <= KeyCode::RIGHT) {
        remove_key(key);
    }
}

void BattleFieldView::procTankControl() {
    auto& gamepad = _game.gamepad();
    std::map<int, int> keyMap = {
            {KeyCode::UP, TankView::UP},
            {KeyCode::DOWN, TankView::DOWN},
            {KeyCode::LEFT, TankView::LEFT},
            {KeyCode::RIGHT, TankView::RIGHT},
    };
    if (_keylist.size()) {
        _player->move(TankView::Direction(keyMap[_keylist.back()]));
    } else {
        _player->stop();
    }
}

void BattleFieldView::pause(bool v) {
    _pause = v;
    if (_pause) {
        _game.event().notify(EasyEvent<Widget*>(EventID::PAUSE_GAME, this));
    } else {
        _game.event().notify(EasyEvent<Widget*>(EventID::RESUME_GAME, this));
    }
}

void BattleFieldView::gameOver() {
    _game.event().notify(EasyEvent<Widget*>(EventID::GAME_OVER, this));
}

void BattleFieldView::add_key(int key) {
    _keylist.push_back(key);
}

bool BattleFieldView::remove_key(int key) {
    auto iter = std::find(_keylist.begin(), _keylist.end(), key);
    if (iter != _keylist.end()) {
        _keylist.erase(iter);
        return true;
    }
    return false;
}

void BattleFieldView::addElement(Widget::Ptr& widget) {
    _root->addChild(widget);

    auto data = (TileData*)widget->userdata();
    if (data->type != TileView::TREES) {
        // 除了树以外的砖块，都加入四叉树进行碰撞处理
        _quadtree->insert(widget);
    }
}

bool sortWidget(Widget::Ptr const& first, Widget::Ptr const& second) {
    auto data_1 = (TileData*)first->userdata();
    auto data_2 = (TileData*)second->userdata();
    assert(data_1 != nullptr and data_2 != nullptr and "BattleFieldView::sortElements fail.");
    return data_1->layer < data_2->layer;
}

void BattleFieldView::sortElements() {
    std::sort(_root->children().begin(), _root->children().end(), sortWidget);
}
