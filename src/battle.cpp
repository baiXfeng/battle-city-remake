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
    _game.event().remove(EventID::TANK_FIRE, this);
    _game.event().remove(EventID::TANK_GEN, this);
    _game.event().remove(EventID::GAME_OVER, this);
}

BattleFieldView::BattleFieldView():
_floor(nullptr),
_root(nullptr),
_upper(nullptr),
_player(nullptr),
_pause(false),
_joyUsed(false),
_world(std::make_shared<WorldModel>()),
_playerModel(std::make_shared<PlayerModel>()) {

    _game.event().add(EventID::BASE_FALL, this);
    _game.event().add(EventID::TANK_FIRE, this);
    _game.event().add(EventID::TANK_GEN, this);
    _game.event().add(EventID::GAME_OVER, this);
    _game.gamepad().sleep(0.0f);

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

    _behavior = Behavior::Ptr(new TankSpawnBehavior(&_world->tanks));
}

void BattleFieldView::onLoadLevel() {
    auto& tile_list = _game.force_get<AddTileList>("add_tile_list");
    auto& tank_list = _game.force_get<AddTankList>("add_tank_list");

    tile_list.clear();
    tank_list.clear();

    auto level = _game.get<int>("level");
    auto file = res::levelName(level);

    auto& state = _game.get<lutok3::State>("lua");
    state.doFile(file);

    _game.set<WorldModel*>("world_model", _world.get());
    _game.set<PlayerModel*>("player_model", _playerModel.get());

    {
        TileBuilder::Array array;
        TileBuilder builder(_world.get());
        builder.gen(array, tile_list);

        for (auto& widget : array) {
            addToBottom(widget);
        }
    }

    _playerModel->win = false;
    _playerModel->life = Tank::getDefaultLifeMax();
    memset(_playerModel->killCount, 0, sizeof(_playerModel->killCount));
}

void BattleFieldView::onUpdate(float delta) {
    procTankControl();
    _behavior->tick(delta);
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
    } else if (key == KeyCode::L1) {
        if (!_pause) {
            pause(true);
        }
        auto widget = Widget::New<CheatView>([&]{
            pause(false);
        });
        _game.screen().scene_back()->addChild(widget);
    } else if (key == KeyCode::A or key == KeyCode::B) {
        _player->fire();
    } else if (key == KeyCode::X) {
        static Tank::Tier tier = Tank::A;
        static bool has_drop = false;
        //_player->setSkin(tier, has_drop);
        int lv = tier + 1;
        if (lv > Tank::Tier::D) {
            lv = tier = Tank::A;
            has_drop = !has_drop;
        }
        tier = Tank::Tier(lv);
        //_player->setSkin(Tank::P1, tier);
        _player->setSkin(tier, has_drop);
    } else if (key == KeyCode::Y) {
        _player->setTopEnemySkin();
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
        addToMiddle(base);

        auto widget = New<BigExplosionView>();
        auto animate = widget->to<BigExplosionView>();
        animate->setAnchor(0.5f, 0.5f);
        animate->setPosition(position);
        animate->play(std::bind(&BattleFieldView::gameOver, this));
        this->addChild(widget);
        widget->performLayout();

    } else if (e.Id() == EventID::TANK_FIRE) {
        // 添加子弹
        auto bullet = e.data<Widget::Ptr>();
        bullet->to<BulletView>()->insert_to(_world.get());
        addToMiddle(bullet);
        bullet->performLayout();

    } else if (e.Id() == EventID::TANK_GEN) {
        // 添加坦克
        auto& info = e.data<TankBuildInfo>();
        auto tank = Widget::Ptr(new TankView(info.party, info.tier, info.direction, info.has_drop, info.controller));
        auto view = tank->to<TankView>();
        view->setBattleField(this);
        view->setPosition(info.position);
        view->turn(info.direction);
        view->insert_to(_world.get());
        addToMiddle(tank);
        if (info.controller == Tank::P1) {
            _player = view;
        }

    } else if (e.Id() == EventID::GAME_OVER) {

        this->gameOver();
    }
}

void BattleFieldView::procTankControl() {
    if (!_player or !_player->visible()) {
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
    _game.event().notify(EasyEvent<Widget*>(EventID::GAME_OVER_ANIMATION, this));
}

void BattleFieldView::addToBottom(Widget::Ptr& widget) {
    _floor->addChild(widget);
}

void BattleFieldView::addToMiddle(Widget::Ptr& widget) {
    _root->addChild(widget);
}

void BattleFieldView::addToTop(Widget::Ptr& widget) {
    _upper->addChild(widget);
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
