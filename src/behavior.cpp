//
// Created by baifeng on 2021/10/9.
//

#include "behavior.h"
#include "common/collision.h"
#include "common/game.h"
#include "common/event.h"
#include "common/audio.h"
#include "const.h"

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
    auto player_model = _game.get<PlayerModel*>("player_model");
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

EnemySpawnBehavior::EnemySpawnBehavior(WorldModel::TankList* tanks):_index(0), _tanks(tanks) {
    _addtanks = &_game.force_get<AddTankList>("add_tank_list");
    _player = &_game.force_get<PlayerModel>("player_model");
    _game.event().notify(EasyEvent<int>(EventID::ENEMY_NUMBER_CHANGED, enemyRemainCount()));
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

int const ENEMY_MAX_COUNT = 4;

Status EnemySpawnBehavior::tick(float delta) {
    if (_player->win) {
        return success;
    }
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

TankAI_Behavior::TankAI_Behavior(TankModel* model):_model(model) {

}

Status TankAI_Behavior::tick(float delta) {
    if (_model->controller != Tank::AI) {
        return running;
    }
    return success;
}

//=====================================================================================

TankMoveBehavior::TankMoveBehavior(TankModel* model, RectI const& bounds):_model(model), _world_bounds(bounds) {
}

Status TankMoveBehavior::tick(float delta) {
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
                _model->position.y = tank->bounds.y + tank->bounds.h;
                _model->move.y = 0.0f;
                _model->modifyPosition();
                break;
            } else if (_model->move.y > 0.0f and _model->bounds.y < tank->bounds.y) {
                _model->position.y = tank->bounds.y - _model->bounds.h;
                _model->move.y = 0.0f;
                _model->modifyPosition();
                break;
            }
            if (_model->move.x < 0.0f and _model->bounds.x > tank->bounds.x) {
                _model->position.x = tank->bounds.x + tank->bounds.w;
                _model->move.x = 0.0f;
                _model->modifyPosition();
                break;
            } else if (_model->move.x > 0.0f and _model->bounds.x < tank->bounds.x) {
                _model->position.x = tank->bounds.x - _model->bounds.w;
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
    _model->removeFromScreen();
    auto iter = std::find(_world->bullets.begin(), _world->bullets.end(), _model);
    if (iter != _world->bullets.end()) {
        _world->bullets.erase(iter);
    }
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

Status BulletTileCollisionBehavior::tick(float delta) {
    WorldModel::TileTree::SquareList list;
    auto& tiles = _world->tiles;
    tiles.retrieve(list, _model->bounds);
    tiles.unique(list, [](TileModel* m) {
        return m;
    });
    auto status = success;
    bool hit_brick = true;
    for (auto& tile : list) {
        if (tile->type == Tile::ICE_FLOOR or tile->type == Tile::TREES or tile->type == Tile::WATERS) {
            continue;
        }
        if (isCollision(_model->bounds, tile->bounds)) {
            if (tile->type == Tile::BASE) {
                // 基地击破，GameOver
                _game.event().notify(EasyEvent<Vector2f>(EventID::BASE_FALL, {
                        float(tile->bounds.x + (tile->bounds.w >> 1)),
                        float(tile->bounds.y + (tile->bounds.h >> 1)),
                }));
                this->hit_base();
                tiles.remove(tile);
                tile->removeFromScreen();
                this->remove_bullet();
                return fail;
            }
            if (tile->type == Tile::STEEL) {
                hit_brick = false;
            }
            tiles.remove(tile);
            tile->removeFromScreen();
            status = fail;
        }
    }
    if (status == fail) {
        this->bullet_explosion();
        this->remove_bullet();
        if (hit_brick) {
            this->hit_brick();
        } else {
            this->hit_wall();
        }
    }
    return status;
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

            if (_model->party == Tank::PLAYER and tank->party == Tank::ENEMY) {
                // 记录击败的敌人
                auto& model = _game.get<PlayerModel>("player_model");
                model.killCount[tank->tier] += 1;
            }

            BulletHitTankInfo info;
            info.bullet = _model;
            info.tank = tank;
            info.world = _world;
            _game.event().notify(EasyEvent<BulletHitTankInfo>(EventID::BULLET_HIT_TANK, info));
            bullet_explosion();
            remove_bullet();

            return fail;
        }
    }
    return success;
}
