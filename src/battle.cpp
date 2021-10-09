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
#include "object.h"
#include "const.h"

//=====================================================================================

static RectI tileRectCatcher(QuadTree<TileModel*>::Square const& square) {
    return square->bounds;
}

BattleFieldView::BattleFieldView():
_root(nullptr),
_player(nullptr),
_pause(false),
_world(std::make_shared<WorldModel>()) {

    auto old_size = this->size();
    this->setSize(Tile::MAP_SIZE, Tile::MAP_SIZE);

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
    //sortElements();
}

void BattleFieldView::onLoadLevel() {
    auto& tile_list = _game.force_get<AddTileList>("add_tile_list");
    tile_list.clear();

    auto level = _game.get<int>("level");
    auto file = res::levelName(level);

    auto& state = _game.get<lutok3::State>("lua");
    state.doFile(file);

    _game.remove("world_model");
    _game.set<WorldModel*>("world_model", _world.get());
    _world->root = _root;

    {
        TileBuilder::Array array;
        TileBuilder builder(_world.get());
        builder.gen(array, tile_list);

        for (auto& widget : array) {
            addElement(widget);
        }
    }

    {
        TankBuilder::Array array;
        TankBuilder builder(_world.get());
        builder.gen(array, TankBuilder::TankType::PLAYER_1, {4.5f * Tile::SIZE, 12.0f * Tile::SIZE});
        for (auto& widget : array) {
            addElement(widget);
        }
        _player = array.front()->to<TankView>();
    }
}

void BattleFieldView::onUpdate(float delta) {
    procTankControl();
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
    //_quadtree->draw(renderer, _global_position.to<int>());
}

void BattleFieldView::onButtonDown(int key) {
    if (key >= KeyCode::UP and key <= KeyCode::RIGHT) {
        add_key(key);
    } else if (key == KeyCode::START) {
        pause(!_pause);
    } else if (key == KeyCode::SELECT) {
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
    return;
    auto data = (TileData*)widget->userdata();
    if (data->type != TileView::TREES) {
        // 除了树以外的砖块，都加入四叉树进行碰撞处理
        //_quadtree->insert(widget);
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
