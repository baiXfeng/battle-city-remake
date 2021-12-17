//
// Created by baifeng on 2021/10/9.
//

#include "behaviors.h"
#include "common/collision.h"
#include "common/game.h"
#include "common/event.h"
#include "common/action.h"
#include "const.h"
#include "view.h"
#include "object.h"
#include "debug.h"
#include "sound_effect.h"

using namespace mge;

typedef mge::Behavior::Status Status;

//=====================================================================================

TankSpawnBehavior::TankSpawnBehavior(WorldModel::TankList* tanks) {
    add(Behavior::Ptr(new PlayerSpawnBehavior(tanks)));
    add(Behavior::Ptr(new EnemySpawnBehavior(tanks)));
}

Status TankSpawnBehavior::tick(float delta) {
    return SequenceBehavior::tick(delta);
}

//=====================================================================================

PlayerSpawnBehavior::PlayerSpawnBehavior(WorldModel::TankList* tanks):_tanks(tanks) {

}

Status PlayerSpawnBehavior::tick(float delta) {
    for (auto& tank : *_tanks) {
        if (tank->party == Tank::PLAYER and tank->controller == Tank::P1) {
            // 玩家坦克已经存在，不再创建
            return running;
        }
    }
    auto player_model = &_game.get<PlayerModel>("player_model");
    if (player_model->life <= 0) {
        return running;
    }
    // 减少生命
    _game.event().notify(EasyEvent<int>(EventID::PLAYER_LIFE_CHANGED, --player_model->life));
    // 创建坦克
    auto& spawns = Tank::getSpawns(Tank::PLAYER);
    TankBuildInfo info;
    info.position = spawns[Tank::P1];
    info.direction = Tank::Direction::UP;
    info.party = Tank::PLAYER;
    info.tier = Tank::A;
    info.controller = Tank::P1;
    info.has_drop = false;
    _game.event().notify(EasyEvent<TankBuildInfo>(EventID::TANK_GEN, info));
    return success;
}

//=====================================================================================

TankEventBehavior::TankEventBehavior(WorldModel::TileTree* tiles): _tiles(tiles), _openingBgmFinished(false) {
    _game.event().add(EventID::PLAYER_STANDBY, this);
    _game.event().add(EventID::PLAYER_MOVE, this);
    _game.audio().addListener(res::soundName("stage_start"), this);
}

TankEventBehavior::~TankEventBehavior() {
    _game.event().remove(EventID::PLAYER_STANDBY, this);
    _game.event().remove(EventID::PLAYER_MOVE, this);
    _game.audio().removeListener(res::soundName("stage_start"), this);
}

Status TankEventBehavior::tick(float delta) {
    return success;
}

void TankEventBehavior::onEvent(Event const& e) {
    if (e.Id() == EventID::PLAYER_STANDBY) {
        if (_openingBgmFinished) {
            _SE.playSE(_SE.TANK_IDLE_SE);
        }
        auto tank = e.data<TankModel*>();
        WorldModel::TileTree::SquareList list;
        _tiles->retrieve(list, tank->bounds);
        for (auto& tile : list) {
            if (tile->type != Tile::ICE_FLOOR) {
                continue;
            }
            if (isCollision(tile->bounds, tank->bounds)) {
                tank->onIceFloor();
                return;
            }
        }
    } else if (e.Id() == EventID::PLAYER_MOVE) {
        if (_openingBgmFinished) {
            _SE.playSE(_SE.TANK_MOVE_SE);
        }
    }
}

void TankEventBehavior::onMixFinished(std::string const& name) {
    auto& world = _game.get<WorldModel>("world_model");
    if (world.player[0]->moving) {
        _SE.playSE(_SE.TANK_MOVE_SE);
    } else {
        _SE.playSE(_SE.TANK_IDLE_SE);
    }
    _openingBgmFinished = true;
}

//=====================================================================================

EnemySpawnBehavior::EnemySpawnBehavior(WorldModel::TankList* tanks):_index(0), _tanks(tanks), _delay_tick(0.0f) {
    _delay = Tank::getGlobalFloat("ENEMY_SPAWN_DELAY");
    _delay_tick = _delay;
    _addtanks = &_game.force_get<AddTankList>("add_tank_list");
    _player = &_game.force_get<PlayerModel>("player_model");
    _game.event().notify(EasyEvent<int>(EventID::ENEMY_NUMBER_CHANGED, enemyRemainCount()));
}

EnemySpawnBehavior::~EnemySpawnBehavior() {
}

int EnemySpawnBehavior::enemyCount() const {
    int enemyCount = 0;
    for (auto& tank : *_tanks) {
        tank->party == Tank::ENEMY ? ++enemyCount : 0;
    }
    return enemyCount;
}

int EnemySpawnBehavior::enemyRemainCount() const {
    return _addtanks->size() - _index;
}

bool EnemySpawnBehavior::is_overlap(RectI const& r) const {
    for (auto& tank : *_tanks) {
        if (isCollision(r, tank->bounds)) {
            return true;
        }
    }
    return false;
}

void EnemySpawnBehavior::checkOverlap(int& index, int& overlapCount) const {
    auto& spawns = Tank::getSpawns(Tank::ENEMY);
    for (int i = 0; i < spawns.size(); ++i) {
        auto& position = spawns[ index ];
        if (is_overlap({
            int(position.x),
            int(position.y),
            Tile::SIZE,
            Tile::SIZE,
        })) {
            ++overlapCount;
            index = ++index % spawns.size();
            continue;
        }
        break;
    }
}

Status EnemySpawnBehavior::tick(float delta) {
    if (_player->win) {
        return success;
    }
    int const ENEMY_MAX_COUNT = 4;
    int enemy_count = enemyCount();
    if (enemy_count >= ENEMY_MAX_COUNT) {
        // 同屏敌军坦克数量超过限制，不再生产
        return running;
    }
    int enemy_remain_count = enemyRemainCount();
    if (enemy_count == 0 and enemy_remain_count == 0) {
        // 屏幕没有敌人，并且不再生产敌人，玩家胜利
        _player->win = true;
        _game.event().notify(Event(EventID::PLAYER_WIN));
        return success;
    }
    if (enemy_remain_count == 0) {
        // 所有坦克生产完毕，不再生产
        return running;
    }
    if ((_delay_tick += delta) < _delay) {
        return running;
    }
    // 检查出生点是否有坦克占据
    auto& spawns = Tank::getSpawns(Tank::ENEMY);
    int index = rand() % spawns.size();
    int overlapCount = 0;
    this->checkOverlap(index, overlapCount);

    if (overlapCount >= spawns.size()) {
        // 所有出生点均有坦克占据
        return running;
    }

    // 生产一个坦克，同时更新剩余敌军坦克数量
    auto& addtank = (*_addtanks)[_index++];
    _game.event().notify(EasyEvent<int>(EventID::ENEMY_NUMBER_CHANGED, enemyRemainCount()));

    // 创建坦克
    TankBuildInfo info;
    info.position = spawns[ index ];
    info.direction = Tank::Direction::DOWN;
    info.party = Tank::ENEMY;
    info.tier = addtank.tier;
    info.controller = Tank::AI;
    info.has_drop = addtank.has_drop;
    _game.event().notify(EasyEvent<TankBuildInfo>(EventID::TANK_GEN, info));

    // 重新计时
    _delay_tick = 0.0f;

    return success;
}

//=====================================================================================

PropCreateBehavior::PropCreateBehavior(WorldModel* world, BattleFieldInterface* battlefield):
_battlefield(battlefield),
_world(world) {
    _game.event().add(EventID::PROP_GEN, this);
}

PropCreateBehavior::~PropCreateBehavior() {
    _game.event().remove(EventID::PROP_GEN, this);
}

Status PropCreateBehavior::tick(float delta) {
    return success;
}

void PropCreateBehavior::onEvent(Event const& e) {
    if (e.Id() == EventID::PROP_GEN) {

        // 保证同屏最多只有一个奖励
        while (_world->props.size()) {
            auto iter = _world->props.begin();
            (*iter)->removeFromScreen();
            _world->props.erase(iter);
        }

        // 创建奖励
        auto type = Tank::PowerUp(rand() % Tank::POWER_MAX);
        auto& cheat = Tank::getCheat();
        if (cheat.appear_powerup_type != Tank::POWER_MAX) {
            type = cheat.appear_powerup_type;
        }
        auto view = Widget::New<PropView>(type);
        auto prop = view->to<PropView>();
        prop->setBattleField(_battlefield);
        prop->insert_to(_world);
        _battlefield->addToTop(view);

        // 设置坐标
        Vector2i position;
        this->randomPosition(position, 5);
        view->setPosition(position.to<float>());
        view->performLayout();

        // 闪烁动画
        auto blink = Action::Ptr(new Blink(prop, 4, 1.2f));
        auto repeat = Action::New<Repeat>(blink);
        view->runAction(repeat);

        // 播放音效
        _SE.playSE(_SE.POWERUP_APPEAR);
    }
}

void PropCreateBehavior::randomPosition(Vector2i& position, int retryCount) {
    int x = rand() % (Tile::MAP_SIZE - Tile::SIZE * 3) + Tile::SIZE;
    int y = rand() % (Tile::MAP_SIZE - Tile::SIZE * 3) + Tile::SIZE;
    int const size = Tile::SIZE >> 1;
    position = {
            x - (x % size),
            y - (y % size),
    };
    if (retryCount <= 0) {
        //printf("奖励坐标重设超过最大重试次数.\n");
        return;
    }
    auto& tiles = _world->tiles;
    WorldModel::TileTree::SquareList result;
    tiles.retrieve(result, {
        position.x,
        position.y,
        Tile::SIZE,
        Tile::SIZE,
    }, [](TileModel* m){
        return m;
    });
    Vector2i offset[4] = {
            {0, 0},
            {size, 0},
            {0, size},
            {size, size},
    };
    int overlapCount = 0;
    for (int i = 0; i < 4; ++i) {
        RectI bounds{
            position.x + offset[i].x,
            position.y + offset[i].y,
            size, size,
        };
        for (auto& tile : result) {
            if (tile->type == Tile::WATERS or tile->type == Tile::STEEL) {
                if (isCollision(tile->bounds, bounds)) {
                    overlapCount++;
                    break;
                }
            }
        }
    }

    if (overlapCount >= 4) {
        //printf("奖励坐标重设[%d, %d].\n", position.x, position.y);
        randomPosition(position, --retryCount);
    }
}

//=====================================================================================

TankPowerUpBehavior::TankPowerUpBehavior(WorldModel* world, BattleFieldInterface* battlefield):_world(world), _battlefield(battlefield) {
    _game.event().add(EventID::TANK_POWERUP, this);
}

TankPowerUpBehavior::~TankPowerUpBehavior() {
    _game.event().remove(EventID::TANK_POWERUP, this);
}

Status TankPowerUpBehavior::tick(float delta) {
    return success;
}

void TankPowerUpBehavior::onEvent(Event const& e) {
    if (e.Id() == EventID::TANK_POWERUP) {

        auto& info = e.data<TankPowerUpInfo>();
        int type = info.prop->type;

        if (type == Tank::STAR) {

            // 坦克升级
            auto tank = info.tank;
            if (tank->tier != Tank::D) {
                auto tier = Tank::Tier(info.tank->tier + 1);
                auto& attr = Tank::getAttribute(tank->party, tier);
                tank->hp = attr.health;
                tank->tier = tier;
            }
            if (tank->party == Tank::PLAYER) {
                // 保留玩家坦克等级，进入下一关时还原
                _game.get<PlayerModel>("player_model").tier = tank->tier;
            }

        } else if (type == Tank::TANK) {

            // 奖命
            auto& player = _game.get<PlayerModel>("player_model");
            _game.event().notify(EasyEvent<int>(EventID::PLAYER_LIFE_CHANGED, ++player.life));

        } else if (type == Tank::TIMER) {

            // 时停
            auto scene = _game.screen().scene_back();
            auto& world = _game.get<WorldModel>("world_model");
            float duration = Tank::getPowerUpDuration("FREEZE");
            world.sleep = true;
            scene->defer([]{
                auto& world = _game.get<WorldModel>("world_model");
                world.sleep = false;
            }, duration);

        } else if (type == Tank::GRENADE) {

            // 炸弹
            auto& world = _game.get<WorldModel>("world_model");
            auto& tanks = world.tanks;
            for (auto iter = tanks.begin(); iter != tanks.end();) {
                auto tank = *iter;
                if (tank->party == Tank::ENEMY) {
                    tanks.erase(iter++);
                    tank->createExplosion();
                    tank->removeFromScreen();
                    continue;
                }
                iter++;
            }
            _SE.playSE(_SE.TANK_EXPLOSION);

        } else if (type == Tank::HELMET) {

            // 护罩
            auto tank = info.tank;
            tank->openShield(Tank::getPowerUpDuration("SHIELD"));

        } else if (type == Tank::SHOVEL) {

            // 基地加固
            float shovel_duration = Tank::getPowerUpDuration("SHOVEL");
            auto scene = _game.screen().scene_back();
            scene->defer([]{
                _game.event().notify(Event(EventID::BASE_REINFORCE_FINISH));
            }, shovel_duration);
            _game.event().notify(Event(EventID::BASE_REINFORCE_START));
        }

        info.prop->createScore();
        Tank::playerScoreAdd( 500 );
    }
}

//=====================================================================================

BaseReinforceBehavior::BaseReinforceBehavior(WorldModel* world, BattleFieldInterface* battlefield):
_steel(false),
_world(world),
_battlefield(battlefield) {
    _game.event().add(EventID::BASE_REINFORCE_START, this);
    _game.event().add(EventID::BASE_REINFORCE_FINISH, this);
}

BaseReinforceBehavior::~BaseReinforceBehavior() {
    _game.event().remove(EventID::BASE_REINFORCE_START, this);
    _game.event().remove(EventID::BASE_REINFORCE_FINISH, this);
}

Status BaseReinforceBehavior::tick(float delta) {
    return success;
}

void BaseReinforceBehavior::onEvent(Event const& e) {

    std::string const actionName = "BaseReinforceBehavior:blink";

    if (e.Id() == EventID::BASE_REINFORCE_START) {

        _steel = true;
        this->onBlinkImp();
        auto scene = _game.screen().scene_back();
        scene->stopAction(actionName);

    } else if (e.Id() == EventID::BASE_REINFORCE_FINISH) {

        auto blink = Action::New<CallBackVoid>(std::bind(&BaseReinforceBehavior::onBlink, this));
        auto delay = Action::New<Delay>(0.25f);
        auto actions = Action::Ptr(new Sequence({blink, delay}));
        auto repeat = Action::New<Repeat>(actions, 14);
        auto finish = Action::New<CallBackVoid>(std::bind(&BaseReinforceBehavior::onFinish, this));
        auto action = Action::Ptr(new Sequence({repeat, finish}));
        auto scene = _game.screen().scene_back();
        action->setName(actionName);
        scene->runAction(action);

    }
}

void BaseReinforceBehavior::onBlink() {
    _steel = !_steel;
    onBlinkImp();
}

void BaseReinforceBehavior::onBlinkImp() {
    auto& tiles = _world->tiles;
    auto& base = _world->base;
    RectI bounds{
            base->bounds.x - Tile::SIZE,
            base->bounds.y - Tile::SIZE,
            base->bounds.w + (Tile::SIZE >> 1),
            base->bounds.h + (Tile::SIZE >> 1),
    };
    WorldModel::TileTree::SquareList result;
    tiles.retrieve(result, bounds, [](TileModel* m){
        return m;
    });
    // 移除基地周围砖块
    for (auto& tile : result) {
        if (tile->type == Tile::BASE) {
            // 保留基地
            continue;
        }
        tiles.remove(tile);
        tile->removeFromScreen();
    }
    // 在基地周围添加砖块
    auto size = Tile::SIZE >> 1;
    Vector2i position = {
            base->bounds.x - size,
            base->bounds.y - size,
    };
    auto type = _steel ? Tile::STEEL : Tile::BRICK;
    Vector2i offset[8] = {
            {size * 0, 0},
            {size * 1, 0},
            {size * 2, 0},
            {size * 3, 0},
            {0, size * 1},
            {0, size * 2},
            {size * 3, size * 1},
            {size * 3, size * 2},
    };
    TileBuilder builder(_world);
    TileBuilder::Array widgets;
    AddTileList add_tile_list;
    for (int i = 0; i < 8; ++i) {
        add_tile_list.push_back({
            type,
            int(position.x + offset[i].x),
            int(position.y + offset[i].y),
        });
    }
    builder.gen(widgets, add_tile_list);
    for (auto& widget : widgets) {
        _battlefield->addToMiddle(widget);
    }
}

void BaseReinforceBehavior::onFinish() {
    _steel = false;
    onBlinkImp();
}

//=====================================================================================

TankAI_Behavior::TankAI_Behavior(TankModel* model):
_model(model),
_shootTicks(1.0f),
_moveTicks(1.0f) {
    _world = &_game.get<WorldModel>("world_model");
}

Status TankAI_Behavior::tick(float delta) {
    if (_model->controller != Tank::AI) {
        return running;
    }
    if (_model->party == Tank::ENEMY and _world->sleep) {
        return running;
    }
    auto& cheat = Tank::getCheat();
    if (not cheat.enemy_misfire) {
        onAiShoot(delta);
    }
    onAiMove(delta);
    return success;
}

void TankAI_Behavior::onAiShoot(float delta) {
    if ((_shootTicks += delta) < 0.25f) {
        return;
    }
    _shootTicks = 0;
    if (rand() % 100 < 40) {
        _model->fire = true;
    }
}

Tank::Direction _randomDir(std::vector<Tank::Direction> const& dirs) {
    return dirs[ rand() % dirs.size() ];
}

void TankAI_Behavior::onAiMove(float delta) {
    float const interval = 0.33f;
    if ((_moveTicks += delta) < interval) {
        return;
    }
    _moveTicks = 0;
    if (rand() % 100 >= 60) {
        return;
    }
    typedef Tank::Direction Direction;
    int n = rand() % 100;
    auto base = _world->base;
    auto dir = _randomDir({Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT});

    if (base != nullptr) {
        int self_y = int(_model->position.y);
        if (base->bounds.y > self_y) {
            dir = Direction::DOWN;
            if (n < 40) {
                dir = _randomDir({Direction::UP, Direction::LEFT, Direction::RIGHT});
            }
        } else if (base->bounds.y == self_y) {
            int self_x = int(_model->position.x);
            if (base->bounds.x < self_x) {
                dir = Direction::LEFT;
                if (n < 40) {
                    dir = _randomDir({Direction::UP, Direction::DOWN, Direction::RIGHT});
                }
            } else if (base->bounds.x > self_x) {
                dir = Direction::RIGHT;
                if (n < 40) {
                    dir = _randomDir({Direction::UP, Direction::LEFT, Direction::DOWN});
                }
            }
        }
    }

    _model->modifyDir(dir);
}

//=====================================================================================

TankMoveBehavior::TankMoveBehavior(TankModel* model, RectI const& bounds):_model(model), _world_bounds(bounds) {
    _world = &_game.get<WorldModel>("world_model");
}

Status TankMoveBehavior::tick(float delta) {
    if (_model->party == Tank::ENEMY and _world->sleep) {
        return running;
    }
    _model->position += _model->move * delta;
    Vector2i const& size = _model->size;
    if (_model->position.y < _world_bounds.y and _model->move.y < 0.0f) {
        _model->position.y = _world_bounds.y;
        _model->move.y = 0.0f;
    } else if (_model->position.y > _world_bounds.h - _model->size.y and _model->move.y > 0.0f) {
        _model->position.y = _world_bounds.h - _model->size.y;
        _model->move.y = 0.0f;
    }
    if (_model->position.x < _world_bounds.x and _model->move.x < 0.0f) {
        _model->position.x = _world_bounds.x;
        _model->move.x = 0.0f;
    } else if (_model->position.x > _world_bounds.w - _model->size.x and _model->move.x > 0.0f) {
        _model->position.x = _world_bounds.w - _model->size.x;
        _model->move.x = 0.0f;
    }
    _model->bounds.x = _model->position.x;
    _model->bounds.y = _model->position.y;
    _model->bounds.w = _model->size.x;
    _model->bounds.h = _model->size.y;
    _model->modifyPosition();
    return success;
}

//=====================================================================================

TankTileCollisionBehavior::TankTileCollisionBehavior(TankModel* model, WorldModel::TileTree* tiles):
_model(model),
_tiles(tiles) {

}

Status TankTileCollisionBehavior::tick(float delta) {
    WorldModel::TileTree::SquareList list;
    _tiles->retrieve(list, _model->bounds);
    for (auto& tile : list) {
        if (not tile->visible) {
            continue;
        }
        if (tile->type == Tile::ICE_FLOOR or tile->type == Tile::TREES) {
            continue;
        }
        if (isCollision(_model->bounds, tile->bounds)) {
            if (_model->move.y < 0.0f and _model->bounds.y > tile->bounds.y) {
                _model->position -= _model->move * delta;
                _model->move.y = 0.0f;
                _model->modifyPosition();
                break;
            } else if (_model->move.y > 0.0f and _model->bounds.y < tile->bounds.y) {
                _model->position -= _model->move * delta;
                _model->move.y = 0.0f;
                _model->modifyPosition();
                break;
            }
            if (_model->move.x < 0.0f and _model->bounds.x > tile->bounds.x) {
                _model->position -= _model->move * delta;
                _model->move.x = 0.0f;
                _model->modifyPosition();
                break;
            } else if (_model->move.x > 0.0f and _model->bounds.x < tile->bounds.x) {
                _model->position -= _model->move * delta;
                _model->move.x = 0.0f;
                _model->modifyPosition();
                break;
            }
        }
    }
    return success;
}

//=====================================================================================

TankCollisionBehavior::TankCollisionBehavior(TankModel* model, WorldModel::TankList* tanks):
_model(model),
_tanks(tanks) {
    _world_bounds = _game.get<WorldModel>("world_model").bounds;
}

Status TankCollisionBehavior::tick(float delta) {
    for (auto& tank : *_tanks) {
        if (tank->id == _model->id) {
            continue;
        }
        if (!tank->visible) {
            continue;
        }
        if (isCollision({
            tank->bounds.x + 5,
            tank->bounds.y + 5,
            tank->bounds.w - 10,
            tank->bounds.h - 10,
            }, _model->bounds)) {
            if (_model->move.y < 0.0f and _model->bounds.y > tank->bounds.y) {
                _model->position -= _model->move * delta;
                _model->move.y = 0.0f;
                _model->modifyPosition();
                break;
            } else if (_model->move.y > 0.0f and _model->bounds.y < tank->bounds.y) {
                _model->position -= _model->move * delta;
                _model->move.y = 0.0f;
                _model->modifyPosition();
                break;
            }
            if (_model->move.x < 0.0f and _model->bounds.x > tank->bounds.x) {
                _model->position -= _model->move * delta;
                _model->move.x = 0.0f;
                _model->modifyPosition();
                break;
            } else if (_model->move.x > 0.0f and _model->bounds.x < tank->bounds.x) {
                _model->position -= _model->move * delta;
                _model->move.x = 0.0f;
                _model->modifyPosition();
                break;
            }
        }
    }
    return success;
}

//=====================================================================================

TankFireBehavior::TankFireBehavior(TankModel* model, WorldModel::BulletList* bullets):
_model(model),
_bullets(bullets) {

}

Status TankFireBehavior::tick(float delta) {
    if (not _model->fire) {
        return running;
    }
    _model->fire = false;
    int bulletCount = 0;
    for (auto& bullet : *_bullets) {
        if (bullet->sender_id == _model->id) {
            // 如果发射过子弹，计数+1
            ++bulletCount;
        }
    }
    auto& attr = Tank::getAttribute(_model->party, _model->tier);
    if (bulletCount >= attr.bulletMaxCount) {
        return running;
    }
    _model->createBullet();
    return success;
}

//=====================================================================================

BulletMoveBehavior::BulletMoveBehavior(BulletModel* model):_model(model) {

}

Status BulletMoveBehavior::tick(float delta) {
    _model->position += _model->move * delta;
    _model->modifyPosition();
    return success;
}

//=====================================================================================

BaseBulletCollisionBehavior::BaseBulletCollisionBehavior(BulletModel* model, WorldModel* world):
_model(model),
_world(world) {

}

void BaseBulletCollisionBehavior::remove_bullet() {
    auto iter = std::find(_world->bullets.begin(), _world->bullets.end(), _model);
    if (iter != _world->bullets.end()) {
        _world->bullets.erase(iter);
    }
    _model->removeFromScreen();
}

void BaseBulletCollisionBehavior::bullet_explosion() {
    _model->playExplosion();
}

void BaseBulletCollisionBehavior::hit_wall() {
    if (_model->party == Tank::ENEMY) {
        return;
    }
    _SE.playSE(_SE.BULLET_HIT_WALL_SE);
}

void BaseBulletCollisionBehavior::hit_brick() {
    if (_model->party == Tank::ENEMY) {
        return;
    }
    _SE.playSE(_SE.BULLET_HIT_BRICK_SE);
}

void BaseBulletCollisionBehavior::hit_base() {
    _SE.playSE(_SE.BASE_EXPLOSION);
}

void BaseBulletCollisionBehavior::hit_tank() {
    _SE.playSE(_SE.TANK_EXPLOSION);
}

//=====================================================================================

BulletWorldCollisionBehavior::BulletWorldCollisionBehavior(BulletModel* model, WorldModel* world):
BaseBulletCollisionBehavior(model, world) {

}

Status BulletWorldCollisionBehavior::tick(float delta) {
    if (_model->move.x < 0.0f and _model->position.x <= _world->bounds.x) {
        bullet_explosion();
        remove_bullet();
        hit_wall();
        return fail;
    } else if (_model->move.x > 0.0f and _model->position.x + _model->bounds.w >= _world->bounds.x + _world->bounds.w) {
        bullet_explosion();
        remove_bullet();
        hit_wall();
        return fail;
    }
    if (_model->move.y < 0.0f and _model->position.y <= _world->bounds.y) {
        bullet_explosion();
        remove_bullet();
        hit_wall();
        return fail;
    } else if (_model->move.y > 0.0f and _model->position.y + _model->bounds.h >= _world->bounds.y + _world->bounds.h) {
        bullet_explosion();
        remove_bullet();
        hit_wall();
        return fail;
    }
    return success;
}

//=====================================================================================

BulletTileCollisionBehavior::BulletTileCollisionBehavior(BulletModel* model, WorldModel* world):
BaseBulletCollisionBehavior(model, world) {

}

RectI BulletTileCollisionBehavior::getBulletBounds() const {
    auto horizontal = int(abs(_model->move.x)) != 0 ? true : false;
    auto width = horizontal ? 2 : Tile::SIZE;
    auto height = horizontal ? Tile::SIZE : 2;
    Vector2i center{
        _model->bounds.x + (_model->bounds.w >> 1),
        _model->bounds.y + (_model->bounds.h >> 1),
    };
    return {
        center.x - (width >> 1),
        center.y - (height >> 1),
        width,
        height,
    };
}

RectI BulletTileCollisionBehavior::getBigBulletBounds() const {
    auto horizontal = int(abs(_model->move.x)) != 0 ? true : false;
    auto width = horizontal ? 2 + (Tile::SIZE >> 1) : Tile::SIZE;
    auto height = horizontal ? Tile::SIZE : 2 + (Tile::SIZE >> 1);
    Vector2i center{
            _model->bounds.x + (_model->bounds.w >> 1),
            _model->bounds.y + (_model->bounds.h >> 1),
    };
    return {
            center.x - (width >> 1),
            center.y - (height >> 1),
            width,
            height,
    };
}

Status BulletTileCollisionBehavior::tick(float delta) {
    WorldModel::TileTree::SquareList list;
    auto& tiles = _world->tiles;
    auto bullet_bounds = getBulletBounds();
    tiles.retrieve(list, bullet_bounds, [](TileModel* m) {
        return m;
    });

    auto collision = [&list, &bullet_bounds]{
        std::vector<TileModel*> result;
        for (auto& tile : list) {
            if (not tile->visible) {
                // 砖块隐藏状态不进行碰撞
                continue;
            }
            if (tile->type == Tile::ICE_FLOOR or tile->type == Tile::TREES or tile->type == Tile::WATERS) {
                continue;
            }
            if (isCollision(bullet_bounds, tile->bounds)) {
                result.push_back(tile);
            }
        }
        return result;
    };

    auto result = collision();

    // 如果是玩家顶级坦克子弹，需要扩大子弹检测区域，让子弹一次可以消除两层砖块或者铁块
    if (_model->wall_damage > 1 and result.size()) {
        list.clear();
        bullet_bounds = getBigBulletBounds();
        tiles.retrieve(list, bullet_bounds, [](TileModel* m) {
            return m;
        });
        result = collision();
    }

    bool base_killed = false;
    bool steel_killed = false;
    for (auto& tile : result) {
        if (tile->type == Tile::BASE) {
            auto& cheat = Tank::getCheat();
            if (cheat.base_unmatched) {
                continue;
            }
            base_killed = true;
            _game.event().notify(EasyEvent<Vector2f>(EventID::BASE_FALL, {
                    float(tile->bounds.x + (tile->bounds.w >> 1)),
                    float(tile->bounds.y + (tile->bounds.h >> 1)),
            }));
        } else if (tile->type == Tile::STEEL) {
            if (not _model->destroy_steel) {
                continue;
            }
            steel_killed = true;
        }
        tiles.remove(tile);
        tile->removeFromScreen();
    }

    if (result.size()) {
        if (base_killed) {
            hit_base();
        } else if (steel_killed) {
            hit_wall();
        } else {
            hit_brick();
        }
        bullet_explosion();
        remove_bullet();
        return fail;
    }

    return success;
}

//=====================================================================================

BulletTankCollisionBehavior::BulletTankCollisionBehavior(BulletModel* model, WorldModel* world):
BaseBulletCollisionBehavior(model, world) {

}

Status BulletTankCollisionBehavior::tick(float delta) {
    auto& tanks = _world->tanks;
    for (auto& tank : tanks) {
        if (_model->sender_id == tank->id or !tank->visible) {
            continue;
        }
        if (_model->party == tank->party and _model->party == Tank::ENEMY) {
            // 敌方子弹不对敌方坦克碰撞
            continue;
        }
        if (isCollision(_model->bounds, tank->bounds)) {

            if (tank->party == Tank::ENEMY) {
                bullet_explosion();
            }

            // remove tank
            bulletHitTank(tank);
            // remove bullet
            remove_bullet();

            return fail;
        }
    }
    return success;
}

void BulletTankCollisionBehavior::bulletHitTank(TankModel* tank) {

    auto bullet = _model;
    auto world = _world;

    auto& cheat = Tank::getCheat();
    if (not tank->shield) {
        --tank->hp;

        if (tank->party == Tank::ENEMY and cheat.enemy_unmatched) {
            ++tank->hp;
        } else if (tank->party == Tank::PLAYER and cheat.player_unmatched) {
            ++tank->hp;
        }
    }

    if (tank->party == Tank::ENEMY) {
        if (tank->has_drop or cheat.always_appear_powerup) {
            // 生成奖励
            tank->has_drop = false;
            _game.event().notify(Event(EventID::PROP_GEN));
        }
    }

    if (tank->hp <= 0) {

        auto& player = _game.get<PlayerModel>("player_model");

        if (_model->party == Tank::PLAYER and tank->party == Tank::ENEMY) {
            // 记录击败的敌人
            player.killCount[tank->tier] += 1;
            // 记录得分
            Tank::playerScoreAdd( (tank->tier+1) * 100 );
            tank->createScore();
        }

        if (tank->party == Tank::PLAYER) {
            // 玩家被击毁，等级归零
            player.tier = Tank::A;
            _game.event().notify(EasyEvent<TankModel*>(EventID::PLAYER_DEAD, tank));
        }

        auto& tanks = world->tanks;
        auto iter = std::find(tanks.begin(), tanks.end(), tank);
        if (iter != tanks.end()) {
            tanks.erase(iter);
        }
        tank->createExplosion();
        tank->removeFromScreen();

        if (tank->party == Tank::PLAYER) {
            hit_base();
        } else {
            hit_tank();
        }
    }
}

//=====================================================================================

BulletBulletCollisionBehavior::BulletBulletCollisionBehavior(BulletModel* model, WorldModel* world):
BaseBulletCollisionBehavior(model, world) {

}

Status BulletBulletCollisionBehavior::tick(float delta) {
    auto& bullets = _world->bullets;
    auto bullet = _model;
    auto remove_self = false;
    for (auto iter = bullets.begin(); iter != bullets.end();) {
        if (bullet->id == (*iter)->id) {
            iter++;
            continue;
        }
        if (isCollision(bullet->bounds, (*iter)->bounds)) {
            (*iter)->removeFromScreen();
            bullets.erase(iter);
            remove_self = true;
            break;
        }
        iter++;
    }
    if (remove_self) {
        auto iter = std::find(bullets.begin(), bullets.end(), bullet);
        if (iter != bullets.end()) {
            bullets.erase(iter);
        }
        bullet->removeFromScreen();
        return fail;
    }
    return success;
}

//=====================================================================================

PropCollisionBehavior::PropCollisionBehavior(PropModel* prop, WorldModel::TankList* tanks, WorldModel::PropList* props):
_model(prop),
_tanks(tanks),
_props(props) {

}

Status PropCollisionBehavior::tick(float delta) {
    for (auto& tank : *_tanks) {
        if (tank->party == Tank::ENEMY) {
            continue;
        }
        if (isCollision(_model->bounds, tank->bounds)) {
            auto iter = std::find(_props->begin(), _props->end(), _model);
            if (iter != _props->end()) {
                _props->erase(iter);
            }
            _game.event().notify(EasyEvent<TankPowerUpInfo>(
                    EventID::TANK_POWERUP,
                    tank, _model
            ));
            this->playEffect(_model->type);
            _model->removeFromScreen();
            break;
        }
    }
    return success;
}

void PropCollisionBehavior::playEffect(Tank::PowerUp type) {
    _SE.playSE(Tank::TANK == type ? _SE.LIFEUP_SE : _SE.POWERUP_PICK);
}
