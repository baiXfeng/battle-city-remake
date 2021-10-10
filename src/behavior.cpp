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
    return success;
}

//=====================================================================================

BulletMoveBehavior::BulletMoveBehavior(BulletModel* model, RectI const& bounds):_model(model), _world_bounds(bounds) {

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

BulletCollisionBehavior::BulletCollisionBehavior(BulletModel* model, WorldModel* world):_model(model), _world(world) {
    _calls.push_back(std::bind(&BulletCollisionBehavior::worldCollision, this, std::placeholders::_1));
    _calls.push_back(std::bind(&BulletCollisionBehavior::tileCollision, this, std::placeholders::_1));
    _game.audio().loadEffect(::hit_wall);
    _game.audio().loadEffect(::hit_brick);
    _game.audio().loadEffect(base_explosion);
}

Status BulletCollisionBehavior::tick(float delta) {
    for (auto& call : _calls) {
        if (call(delta) == fail) {
            return fail;
        }
    }
    return success;
}

Status BulletCollisionBehavior::tileCollision(float delta) {
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
                    tile->bounds.x + (tile->bounds.w >> 1),
                    tile->bounds.y + (tile->bounds.h >> 1),
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

Status BulletCollisionBehavior::worldCollision(float delta) {
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

void BulletCollisionBehavior::remove_bullet() {
    _model->removeFromScreen();
    auto iter = std::find(_world->bullets.begin(), _world->bullets.end(), _model);
    if (iter != _world->bullets.end()) {
        _world->bullets.erase(iter);
    }
}

void BulletCollisionBehavior::bullet_explosion() {
    _model->playExplosion();
}

void BulletCollisionBehavior::hit_wall() {
    if (_model->camp == Tank::ENEMY) {
        return;
    }
    _game.audio().playEffect(::hit_wall);
}

void BulletCollisionBehavior::hit_brick() {
    if (_model->camp == Tank::ENEMY) {
        return;
    }
    _game.audio().playEffect(::hit_brick);
}

void BulletCollisionBehavior::hit_base() {
    _game.audio().playEffect(base_explosion);
}
