//
// Created by baifeng on 2021/10/9.
//

#include "behavior.h"
#include "common/collision.h"
#include "common/game.h"
#include "common/event.h"
#include "common/audio.h"
#include "common/action.h"
#include "const.h"
#include "view.h"
#include "object.h"

typedef Behavior::Status Status;

//=====================================================================================

SequenceBehavior::SequenceBehavior() {}

SequenceBehavior::SequenceBehavior(Behaviors const& sequence):_sequence(sequence) {

}

Status SequenceBehavior::tick(float delta) {
    auto status = success;
    for (auto& c : _sequence) {
        if ((status = c->tick(delta)) == fail) {
            break;
        }
    }
    return status;
}

void SequenceBehavior::add(Behavior::Ptr const& behavior) {
    _sequence.push_back(behavior);
}

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

EnemySpawnBehavior::EnemySpawnBehavior(WorldModel::TankList* tanks):_index(0), _tanks(tanks), _delay_tick(0.0f) {
    _delay = Tank::getGlobalFloat("ENEMY_SPAWN_DELAY");
    _delay_tick = _delay;
    _addtanks = &_game.force_get<AddTankList>("add_tank_list");
    _player = &_game.force_get<PlayerModel>("player_model");
    _game.event().notify(EasyEvent<int>(EventID::ENEMY_NUMBER_CHANGED, enemyRemainCount()));
    _game.event().add(EventID::ENEMY_KILLED, this);
}

EnemySpawnBehavior::~EnemySpawnBehavior() {
    _game.event().remove(EventID::ENEMY_KILLED, this);
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

void EnemySpawnBehavior::onEvent(Event const& e) {

    if (e.Id() == EventID::ENEMY_KILLED) {

        _delay_tick = 0.0f;
    }
}

Status EnemySpawnBehavior::tick(float delta) {
    if ((_delay_tick += delta) < _delay) {
        return running;
    }
    _delay_tick = 0.0f;
    return onSpawn(delta);
}

Status EnemySpawnBehavior::onSpawn(float delta) {
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
    return success;
}

//=====================================================================================

PropCreateBehavior::PropCreateBehavior(BattleFieldInterface* battlefield, WorldModel* world):
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
        auto view = Widget::New<PropView>(type);
        auto prop = view->to<PropView>();
        prop->setBattleField(_battlefield);
        prop->insert_to(_world);
        _battlefield->addToTop(view);

        // 设置坐标
        int x = rand() % (Tile::MAP_SIZE - Tile::SIZE * 3) + Tile::SIZE;
        int y = rand() % (Tile::MAP_SIZE - Tile::SIZE * 3) + Tile::SIZE;
        Vector2f position = {
                float(x - (x % (Tile::SIZE >> 1))),
                float(y - (y % (Tile::SIZE >> 1))),
        };
        view->setPosition(position);
        view->performLayout();

        // 闪烁动画
        auto blink = Action::Ptr(new Blink(prop, 4, 1.2f));
        auto repeat = Action::New<Repeat>(blink);
        view->runAction(repeat);

        // 播放音效
        auto sound = res::soundName("powerup_appear");
        _game.audio().loadEffect(sound);
        _game.audio().playEffect(sound);
    }
}

//=====================================================================================

TankPowerUpBehavior::TankPowerUpBehavior() {
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
                tank->modifyTier();
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
            auto sound = res::soundName("explosion_1");
            _game.audio().loadEffect(sound);
            _game.audio().playEffect(sound);

        } else if (type == Tank::HELMET) {

            // 护罩
            auto tank = info.tank;
            tank->openShield(Tank::getPowerUpDuration("SHIELD"));

        } else if (type == Tank::SHOVEL) {

            // 基地加固

        }

        info.prop->createScore();
        Tank::playerScoreAdd( 500 );
    }
}

//=====================================================================================

TankAI_Behavior::TankAI_Behavior(TankModel* model):_model(model) {
    _world = &_game.get<WorldModel>("world_model");
}

Status TankAI_Behavior::tick(float delta) {
    if (_model->controller != Tank::AI) {
        return running;
    }
    if (_model->party == Tank::ENEMY and _world->sleep) {
        return running;
    }
    onAiShoot(delta);
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
    _tiles->unique(list, [](TileModel* m) {
        return m;
    });
    for (auto& tile : list) {
        if (tile->type == Tile::ICE_FLOOR or tile->type == Tile::TREES) {
            continue;
        }
        if (isCollision(_model->bounds, tile->bounds)) {
            if (_model->move.y < 0.0f and _model->bounds.y > tile->bounds.y) {
                _model->position.y = tile->bounds.y + tile->bounds.h;
                _model->move.y = 0.0f;
                _model->modifyPosition();
                break;
            } else if (_model->move.y > 0.0f and _model->bounds.y < tile->bounds.y) {
                _model->position.y = tile->bounds.y - _model->bounds.h;
                _model->move.y = 0.0f;
                _model->modifyPosition();
                break;
            }
            if (_model->move.x < 0.0f and _model->bounds.x > tile->bounds.x) {
                _model->position.x = tile->bounds.x + tile->bounds.w;
                _model->move.x = 0.0f;
                _model->modifyPosition();
                break;
            } else if (_model->move.x > 0.0f and _model->bounds.x < tile->bounds.x) {
                _model->position.x = tile->bounds.x - _model->bounds.w;
                _model->move.x = 0.0f;
                _model->modifyPosition();
                break;
            }
        }
    }
    //printf("碰撞: %d\n", list.size());
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
        if (isCollision({
            tank->bounds.x + 5,
            tank->bounds.y + 5,
            tank->bounds.w - 10,
            tank->bounds.h - 10,
            }, _model->bounds)) {
            if (_model->move.y < 0.0f and _model->bounds.y > tank->bounds.y) {
                if (tank->bounds.y + tank->bounds.h <= _world_bounds.h) {
                    _model->position.y = tank->bounds.y + tank->bounds.h;
                }
                _model->move.y = 0.0f;
                _model->modifyPosition();
                break;
            } else if (_model->move.y > 0.0f and _model->bounds.y < tank->bounds.y) {
                if (tank->bounds.y - _model->bounds.h >= 0) {
                    _model->position.y = tank->bounds.y - _model->bounds.h;
                }
                _model->move.y = 0.0f;
                _model->modifyPosition();
                break;
            }
            if (_model->move.x < 0.0f and _model->bounds.x > tank->bounds.x) {
                if (tank->bounds.x + tank->bounds.w <= _world_bounds.w) {
                    _model->position.x = tank->bounds.x + tank->bounds.w;
                }
                _model->move.x = 0.0f;
                _model->modifyPosition();
                break;
            } else if (_model->move.x > 0.0f and _model->bounds.x < tank->bounds.x) {
                if (tank->bounds.x - _model->bounds.w >= 0) {
                    _model->position.x = tank->bounds.x - _model->bounds.w;
                }
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

std::string hit_wall = res::soundName("bullet_hit_1");
std::string hit_brick = res::soundName("bullet_hit_2");
std::string base_explosion = res::soundName("explosion_2");

BaseBulletCollisionBehavior::BaseBulletCollisionBehavior(BulletModel* model, WorldModel* world):
_model(model),
_world(world) {
    _game.audio().loadEffect(::hit_wall);
    _game.audio().loadEffect(::hit_brick);
    _game.audio().loadEffect(base_explosion);
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
    _game.audio().playEffect(::hit_wall);
}

void BaseBulletCollisionBehavior::hit_brick() {
    if (_model->party == Tank::ENEMY) {
        return;
    }
    _game.audio().playEffect(::hit_brick);
}

void BaseBulletCollisionBehavior::hit_base() {
    _game.audio().playEffect(base_explosion);
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

Status BulletTileCollisionBehavior::tick(float delta) {
    WorldModel::TileTree::SquareList list;
    auto& tiles = _world->tiles;
    auto bullet_bounds = getBulletBounds();
    tiles.retrieve(list, bullet_bounds);
    tiles.unique(list, [](TileModel* m) {
        return m;
    });
    std::vector<TileModel*> result;
    for (auto& tile : list) {
        if (tile->type == Tile::ICE_FLOOR or tile->type == Tile::TREES or tile->type == Tile::WATERS) {
            continue;
        }
        if (isCollision(bullet_bounds, tile->bounds)) {
            result.push_back(tile);
        }
    }

    bool base_killed = false;
    bool steel_killed = false;
    for (auto& tile : result) {
        if (tile->type == Tile::BASE) {
            base_killed = true;
            _game.event().notify(EasyEvent<Vector2f>(EventID::BASE_FALL, {
                    float(tile->bounds.x + (tile->bounds.w >> 1)),
                    float(tile->bounds.y + (tile->bounds.h >> 1)),
            }));
        } else if (tile->type == Tile::STEEL) {
            if (_model->wall_damage == 1) {
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
        if (_model->sender_id == tank->id) {
            continue;
        }
        if (_model->party == tank->party and _model->party == Tank::ENEMY) {
            // 敌方子弹不对敌方坦克碰撞
            continue;
        }
        if (isCollision(_model->bounds, tank->bounds)) {

            if (tank->party == Tank::ENEMY) {
                bullet_explosion();
            } else {
                hit_base();
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

    if (not tank->shield) {
        --tank->hp;
        if (tank->party == Tank::ENEMY and tank->has_drop) {
            // 生成奖励
            _game.event().notify(Event(EventID::PROP_GEN));
        }
    }

    if (tank->hp <= 0) {

        if (_model->party == Tank::PLAYER and tank->party == Tank::ENEMY) {
            // 记录击败的敌人
            auto& model = _game.get<PlayerModel>("player_model");
            model.killCount[tank->tier] += 1;
            // 记录得分
            Tank::playerScoreAdd( (tank->tier+1) * 100 );
        }

        auto& tanks = world->tanks;
        auto iter = std::find(tanks.begin(), tanks.end(), tank);
        if (iter != tanks.end()) {
            tanks.erase(iter);
        }
        tank->createScore();
        tank->createExplosion();
        tank->removeFromScreen();

        auto sound = res::soundName("explosion_1");
        _game.audio().loadEffect(sound);
        _game.audio().playEffect(sound);

        _game.event().notify(Event(EventID::ENEMY_KILLED));
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
        (*iter)->removeFromScreen();
        bullets.erase(iter);
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
    auto sound = res::soundName(type == Tank::TANK ? "life" : "powerup_pick");
    _game.audio().loadEffect(sound);
    _game.audio().playEffect(sound);
}
