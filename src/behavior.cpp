//
// Created by baifeng on 2021/10/9.
//

#include "behavior.h"
#include "common/collision.h"

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
        if (tile->type == Tile::ICE_FLOOR) {
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