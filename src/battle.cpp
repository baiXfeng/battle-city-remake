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
#include "common/log.h"
#include "object.h"
#include "const.h"

//=====================================================================================

static RectI tileRectCatcher(QuadTree<TileModel*>::Square const& square) {
    return square->bounds;
}

BattleFieldView::~BattleFieldView() {
    _game.event().remove(EventID::BASE_FALL, this);
}

BattleFieldView::BattleFieldView():
_floor(nullptr),
_root(nullptr),
_upper(nullptr),
_player(nullptr),
_pause(false),
_joyUsed(false),
_world(std::make_shared<WorldModel>()) {

    _game.event().add(EventID::BASE_FALL, this);

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
        _floor = view.get();
    }

    {
        auto view = New<WindowWidget>();
        view->setSize(size());
        addChild(view);
        _root = view.get();
    }

    {
        auto view = New<Widget>();
        view->setSize(size());
        addChild(view);
        _upper = view.get();
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

    _game.set<WorldModel*>("world_model", _world.get());
    _game.set<std::map<void*, int>>("object_layers");

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
        _player->turn(Tank::Direction::UP);
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
        if (_pause) {
            return;
        }
        gameOver();
    } else if (key == KeyCode::L1) {
        if (!_pause) {
            pause(true);
        }
        auto widget = Widget::New<CheatView>([&]{
            pause(false);
        });
        _game.screen().scene_back()->addChild(widget);
    } else if (key == KeyCode::A or key == KeyCode::B) {
        auto bullet = _player->fire();
        bullet->insert_to(_world.get());
        Ptr widget(bullet);
        addElement(widget);
    }
}

void BattleFieldView::onButtonUp(int key) {
    if (key >= KeyCode::UP and key <= KeyCode::RIGHT) {
        remove_key(key);
    }
}

void BattleFieldView::onJoyAxisMotion(JOYIDX joy_id, int x, int y) {
    if (joy_id != JOY1) {
        return;
    }
    if (x < -70) {
        _joyUsed = true;
        _keylist.clear();
        _keylist.push_back(KeyCode::LEFT);
    } else if (x > 70) {
        _joyUsed = true;
        _keylist.clear();
        _keylist.push_back(KeyCode::RIGHT);
    } else if (y < -70) {
        _joyUsed = true;
        _keylist.clear();
        _keylist.push_back(KeyCode::UP);
    } else if (y > 70) {
        _joyUsed = true;
        _keylist.clear();
        _keylist.push_back(KeyCode::DOWN);
    } else if (abs(x) <= 30 and abs(y) <= 30) {
        if (_joyUsed) {
            _keylist.clear();
            _joyUsed = false;
        }
    }
}

void BattleFieldView::onEvent(Event const& e) {
    if (e.Id() == EventID::BASE_FALL) {
        auto& position = e.data<Vector2f>();
        this->sleep_gamepad(60.0f);

        auto base = New<ImageWidget>(res::load_texture(_game.renderer(), res::imageName("base_destroyed")));
        base->setPosition(position - base->size() * 0.5f);
        addElement(base);

        auto widget = New<BigExplosionView>();
        auto animate = widget->to<BigExplosionView>();
        animate->setAnchor(0.5f, 0.5f);
        animate->setPosition(position);
        animate->play(std::bind(&BattleFieldView::gameOver, this));
        this->addChild(widget);
        widget->performLayout();
    }
}

void BattleFieldView::procTankControl() {
    if (!_player->visible()) {
        return;
    }
    auto& gamepad = _game.gamepad();
    std::map<int, int> keyMap = {
            {KeyCode::UP, TankView::Direction::UP},
            {KeyCode::DOWN, TankView::Direction::DOWN},
            {KeyCode::LEFT, TankView::Direction::LEFT},
            {KeyCode::RIGHT, TankView::Direction::RIGHT},
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
    if (_joyUsed) {
        return;
    }
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
    auto value = getViewLayer(widget.get());
    if (value == 0) {
        _floor->addChild(widget);
    } else if (value == 2) {
        _upper->addChild(widget);
    } else {
        _root->addChild(widget);
    }
}
