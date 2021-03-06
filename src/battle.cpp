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
#include "skin.h"
#include "debug.h"
#include "sound_effect.h"

using namespace mge;

//=====================================================================================

static RectI tileRectCatcher(QuadTree<TileModel*>::Square const& square) {
    return square->bounds;
}

BattleFieldView::~BattleFieldView() {
    _game.event().remove(EventID::BASE_FALL, this);
    _game.event().remove(EventID::TANK_GEN, this);
    _game.event().remove(EventID::GAME_OVER, this);
    _game.event().remove(EventID::PLAYER_DEAD, this);
    _game.event().remove(EventID::PLAYER_STOP_CONTROL, this);
    _SE.stopTankSE();
}

BattleFieldView::BattleFieldView():
_keyCount(0),
_floor(nullptr),
_root(nullptr),
_upper(nullptr),
_player(nullptr),
_pause(false),
_joyUsed(false),
_world(nullptr) {

    _game.event().add(EventID::BASE_FALL, this);
    _game.event().add(EventID::TANK_GEN, this);
    _game.event().add(EventID::GAME_OVER, this);
    _game.event().add(EventID::PLAYER_DEAD, this);
    _game.event().add(EventID::PLAYER_STOP_CONTROL, this);
    _game.gamepad().sleep(10.0f);

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
        auto view = New<WindowWidget>();
        view->setSize(size());
        addChild(view);
        _upper = view.get();
    }

    onLoadLevel();

    auto prop_create = Behavior::Ptr(new PropCreateBehavior(_world, this));
    auto tank_powerup = Behavior::Ptr(new TankPowerUpBehavior(_world, this));
    auto tank_spawn = Behavior::Ptr(new TankSpawnBehavior(&_world->tanks));
    auto tank_event = Behavior::Ptr(new TankEventBehavior(&_world->tiles));
    auto base_reinforce = Behavior::Ptr(new BaseReinforceBehavior(_world, this));
    _behavior = Behavior::Ptr(new SequenceBehavior({prop_create, tank_powerup, tank_spawn, tank_event, base_reinforce}));
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

    _game.set<WorldModel>("world_model");

    _world = &_game.get<WorldModel>("world_model");
    _world->base = nullptr;
    _world->sleep = false;
    memset(_world->player, 0, sizeof(_world->player));

    {
        TileBuilder::Array array;
        TileBuilder builder(_world);
        builder.gen(array, tile_list);

        for (auto& widget : array) {
            auto tile = widget->to<TileView>();
            if (tile->layer() == 0) {
                addToBottom(widget);
            } else if (tile->layer() == 2) {
                addToTop(widget);
            } else {
                addToMiddle(widget);
            }

            if (tile->model()->type == Tile::Type::BASE) {
                _world->base = (TileModel*)tile->model();
            }
        }
    }

    auto& player = _game.force_get<PlayerModel>("player_model");
    player.win = false;
    player.life += 1;
    memset(player.killCount, 0, sizeof(player.killCount));
}

void BattleFieldView::onUpdate(float delta) {
    if (_pause) {
        return;
    }
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
    _world->tiles.draw(renderer, _global_position.to<int>());
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
        if (_player) {
            _player->fire();
        }
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
        addToBottom(base);

        auto widget = New<BigExplosionView>();
        auto animate = widget->to<BigExplosionView>();
        animate->setAnchor(0.5f, 0.5f);
        animate->setPosition(position);
        animate->play(std::bind(&BattleFieldView::gameOver, this));
        this->addChild(widget);
        widget->performLayout();

    } else if (e.Id() == EventID::TANK_GEN) {
        // ????????????
        auto& info = e.data<TankBuildInfo>();
        auto tier = info.tier;
        if (info.controller == Tank::P1) {
            tier = _game.get<PlayerModel>("player_model").tier;
            auto& cheat = Tank::getCheat();
            if (cheat.player_level_max) {
                tier = Tank::D;
            }
        }
        auto tank = Widget::Ptr(new TankView(info.party, tier, info.direction, info.has_drop, info.controller));
        auto view = tank->to<TankView>();
        view->setBattleField(this);
        view->setPosition(info.position);
        view->turn(info.direction);
        view->insert_to(_world);
        addToMiddle(tank);
        if (info.controller == Tank::P1) {
            _player = view;
            _world->player[Tank::Controller::P1] = (TankModel*)_player->model();
        }

        float duration = 1.2f;
        {
            // ????????????
            auto view = New<FrameAnimationWidget>();
            auto animate = view->to<FrameAnimationWidget>();
            animate->setPosition(tank->position());
            animate->setFrames(skin::getTankAppearSkin());
            animate->play(0.4f);
            addChild(view);
            view->defer(animate, [](Widget* sender){
                sender->removeFromParent();
            }, duration);
        }

        view->setVisible(false);
        view->defer(view, [](Widget* sender){
            auto tank = sender->to<TankView>();
            tank->setVisible(true);
            if (tank->model()->party == Tank::PLAYER) {
                // ??????????????????
                tank->open_shield(Tank::getGlobalFloat("PLAYER_SPAWN_SHIELD_DURATION"));
                _game.gamepad().sleep(0.0f);
            }
        }, duration);
        view->pauseAllActionWhenHidden(false);

    } else if (e.Id() == EventID::GAME_OVER) {

        _keylist.clear();
        this->gameOver();

    } else if (e.Id() == EventID::PLAYER_DEAD) {

        _player = nullptr;

        auto& player = _game.get<PlayerModel>("player_model");
        if (player.life == 0) {
            gameOver();
        }

    } else if (e.Id() == EventID::PLAYER_STOP_CONTROL) {

        _keylist.clear();
        _SE.stopTankSE();

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
        if (_keylist.size() != _keyCount) {
            _game.event().notify(Event(EventID::PLAYER_MOVE));
        }
    }
    if (_keylist.empty() and _keylist.size() != _keyCount) {
        _player->stop();
        _game.event().notify(EasyEvent<TankModel*>(EventID::PLAYER_STANDBY, (TankModel*)_player->model()));
    }
    _keyCount = _keylist.size();
}

void BattleFieldView::pause(bool v) {
    _pause = v;
    if (_pause) {
        _keylist.clear();
        _game.event().notify(EasyEvent<Widget*>(EventID::PAUSE_GAME, this));
        _game.audio().pauseMusic();
    } else {
        _game.event().notify(EasyEvent<Widget*>(EventID::RESUME_GAME, this));
        _game.audio().resumeMusic();
    }
}

void BattleFieldView::gameOver() {
    _SE.stopTankSE();
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
