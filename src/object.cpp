//
// Created by baifeng on 2021/10/9.
//

#include "object.h"
#include "common/action.h"
#include "common/loadres.h"
#include "common/game.h"
#include "common/audio.h"
#include "behavior.h"
#include "view.h"
#include "skin.h"
#include "const.h"

static int _objectCount = 0;

//=====================================================================================

class FrameAnimationAction : public Action {
public:
    typedef std::vector<Texture::Ptr> Frames;
public:
    FrameAnimationAction(ImageWidget* target, Frames const& arr, float duration):
            _textures(arr),
            _target(target),
            _index(0),
            _timer(0.0f),
            _timer_max(duration / arr.size()) {}
private:
    State Step(float dt) override {
        if ((_timer += dt) >= _timer_max) {
            _timer -= _timer_max;
            if (++_index >= _textures.size()) {
                _index = 0;
            }
            _target->setTexture(_textures[_index]);
            _target->setSize(Tile::SIZE, Tile::SIZE);
        }
        return RUNNING;
    }
private:
    int _index;
    float _timer;
    float _timer_max;
    ImageWidget* _target;
    Frames _textures;
};

TileView::TileView(TYPE t):
ImageWidget(nullptr),
_type(TYPE_BEGIN) {
    _model.id = ++_objectCount;
    _model.layer = 0;
    _model.type = Tile::TYPE_END;
    _model.add_observer(this);
    setType(t);
}

void TileView::setType(TYPE t) {
    if (_type == t) {
        return;
    }
    Texture::Ptr texture;
    _type = t;
    _model.layer = 1;
    _model.bounds.w = Tile::SIZE;
    _model.bounds.h = Tile::SIZE;
    _action->clear();
    switch (t) {
        case BASE:
            texture = res::load_texture(_game.renderer(), "assets/images/base.png");
            setTexture(texture);
            _model.type = Tile::BASE;
            break;
        case BRICK_0:
        case BRICK_1:
        case BRICK_2:
        case BRICK_3:
        {
            texture = res::load_texture(_game.renderer(), "assets/images/wall_brick.png");
            int half_width = Tile::SIZE >> 1;
            int half_height = Tile::SIZE >> 1;
            SDL_Rect srcrect[4] = {
                    {0, 0, half_width, half_height},
                    {half_width, 0, half_width, half_height},
                    {0, half_height, half_width, half_height},
                    {half_width, half_height, half_width, half_height},
            };
            setTexture(texture, srcrect[t-BRICK_0]);
            setSize(half_width, half_height);
            _model.type = Tile::BRICK;
            _model.bounds.w = Tile::SIZE >> 2;
            _model.bounds.h = Tile::SIZE >> 2;
            return;
        }
            break;
        case STEEL_0:
        case STEEL_1:
        case STEEL_2:
        case STEEL_3:
        {
            texture = res::load_texture(_game.renderer(), "assets/images/wall_steel.png");
            int half_width = Tile::SIZE >> 1;
            int half_height = Tile::SIZE >> 1;
            SDL_Rect srcrect[4] = {
                    {0, 0, half_width, half_height},
                    {half_width, 0, half_width, half_height},
                    {0, half_height, half_width, half_height},
                    {half_width, half_height, half_width, half_height},
            };
            setTexture(texture, srcrect[t-STEEL_0]);
            setSize(half_width, half_height);
            _model.type = Tile::STEEL;
            _model.bounds.w = Tile::SIZE >> 2;
            _model.bounds.h = Tile::SIZE >> 2;
            return;
        }
            break;
        case TREES:
            texture = res::load_texture(_game.renderer(), "assets/images/trees.png");
            setTexture(texture);
            _model.layer = 2;
            _model.type = Tile::TREES;
            break;
        case ICE_FLOOR:
            texture = res::load_texture(_game.renderer(), "assets/images/ice_floor.png");
            setTexture(texture);
            _model.layer = 0;
            _model.type = Tile::ICE_FLOOR;
            break;
        case WATER:
        {
            FrameAnimationAction::Frames frames = {
                    res::load_texture(_game.renderer(), "assets/images/water_1.png"),
                    res::load_texture(_game.renderer(), "assets/images/water_2.png"),
            };
            setTexture(frames[0]);
            runAction(Action::Ptr(new FrameAnimationAction(this, frames, 1.0f)));
            setSize(Tile::SIZE, Tile::SIZE);
            _model.layer = 0;
            _model.type = Tile::WATERS;
            return;
        }
            break;
        default:
            break;
    }
    setSize(Tile::SIZE, Tile::SIZE);
}

int TileView::type() const {
    return _type;
}

TileModel const& TileView::model() const {
    return _model;
}

void TileView::insert_to(WorldModel* world) {
    world->tiles.insert(&_model);
}

void TileView::update(float delta) {
    if (not _action->empty()) {
        _action->update(delta);
    }
    if (_dirty) {
        this->modifyLayout();
    }
}

void TileView::draw(SDL_Renderer* renderer) {
    if (!_visible) {
        return;
    }
    ImageWidget::onDraw(renderer);
}

void TileView::onModifyPosition(Vector2f const& position) {
    _model.bounds.x = position.x;
    _model.bounds.y = position.y;
}

void TileView::onModifySize(Vector2f const& size) {
    _model.bounds.w = size.x;
    _model.bounds.h = size.y;
}

//=====================================================================================

std::string shot_sound = res::soundName("bullet_shot");

TankView::TankView(Tank::Party party, Tank::Tier tier, Tank::Direction dir, bool has_drop, Controller c):_battlefield(nullptr) {
    _model.dir = dir;
    if (party == Tank::ENEMY) {
        setSkin(tier, has_drop);
    } else if (party == Tank::PLAYER) {
        setSkin(c, tier);
    }
    _model.id = ++_objectCount;
    _model.fire = false;
    _model.shield = true;
    _model.party = party;
    _model.controller = c;
    _model.size = {Tile::SIZE, Tile::SIZE};
    _model.dir = Direction::MAX;
    _model.add_observer(this);
    enableUpdate(true);
    _game.audio().loadEffect(shot_sound);
}

void TankView::setSkin(Controller c, Tank::Tier tier) {
    _texArr = skin::getPlayerSkin(tier, c);
    _model.has_drop = false;
    _model.party = Tank::PLAYER;
    _model.tier = tier;
    if (_model.dir != Tank::MAX) {
        updateMoveSpeed();
        setFrames(_texArr[_model.dir]);
        if (moving()) {
            play(0.12f);
        } else {
            turn(_model.dir);
        }
    }
}

void TankView::setSkin(Tank::Tier tier, bool has_drop) {
    _texArr = skin::getEnemySkin(tier, has_drop);
    _model.has_drop = has_drop;
    _model.party = Tank::ENEMY;
    _model.tier = tier;
    if (_model.dir != Tank::MAX) {
        updateMoveSpeed();
        setFrames(_texArr[_model.dir]);
        play(0.12f);
    }
}

void TankView::setTopEnemySkin() {
    _texArr = skin::getTopEnemySkin();
    _model.has_drop = false;
    _model.party = Tank::ENEMY;
    _model.tier = Tank::D;
    if (_model.dir != Tank::MAX) {
        updateMoveSpeed();
        setFrames(_texArr[_model.dir]);
        play(0.12f);
    }
}

void TankView::move(Direction dir) {
    if (!_force_move) {
        if (dir == Direction::MAX or _model.dir == dir) {
            return;
        }
    }
    if (_model.dir != dir) {
        setFrames(_texArr[dir]);
    }
    play(0.12f);
    _force_move = false;
    _model.dir = dir;
    this->updateMoveSpeed();
    this->onChangeDir(dir);
}

void TankView::turn(Direction dir) {
    if (dir < _texArr.size() and _texArr[dir].size()) {
        setTexture(_texArr[dir].front());
    }
    _model.dir = dir;
    _force_move = true;
}

void TankView::stop(Direction dir) {
    _force_move = true;
    if (_model.move == Vector2f{0.0f, 0.0f}) {
        if (_model.party == Tank::PLAYER) {
            FrameAnimationWidget::stop();
        }
        return;
    }
    if (dir == Direction::UP or dir == Direction::DOWN) {
        _model.move.y = 0.0f;
    } else if (dir == Direction::LEFT or dir == Direction::RIGHT) {
        _model.move.x = 0.0f;
    } else {
        _model.move = {0, 0};
    }
}

void TankView::insert_to(WorldModel* world) {
    _model.position = position();
    world->tanks.push_back(&_model);
    auto tank_ai = Behavior::Ptr(new TankAI_Behavior(&_model));
    auto tank_move = Behavior::Ptr(new TankMoveBehavior(&_model, world->bounds));
    auto tank_fire = Behavior::Ptr(new TankFireBehavior(&_model, &world->bullets));
    auto tile_collision = Behavior::New<TankTileCollisionBehavior>(&_model, &world->tiles);
    auto tank_collision = Behavior::New<TankCollisionBehavior>(&_model, &world->tanks);
    _behavior = Behavior::Ptr(new SequenceBehavior({tank_ai, tank_move, tank_fire, tile_collision, tank_collision}));
}

void TankView::fire() {
    _model.fire = true;
}

void TankView::onFire() {
    Widget::Ptr bullet(createBullet());
    _game.event().notify(EasyEvent<Widget::Ptr>(EventID::TANK_FIRE, bullet));
}

BulletView* TankView::createBullet() const {
    _game.audio().playEffect(shot_sound);
    Vector2f offset[4] = {
            {size().x * 0.5f, 0.0f},
            {size().x * 1.0f, size().y * 0.5f},
            {size().x * 0.5f, size().y * 1.0f},
            {0.0f, size().y * 0.5f},
    };
    auto& attr = Tank::getAttribute(_model.party, _model.tier);
    auto bulletSpeed = attr.bulletSpeed;
    Vector2f speed[4] = {
            {0.0f, -bulletSpeed},
            {bulletSpeed, 0.0f},
            {0.0f, bulletSpeed},
            {-bulletSpeed, 0.0f},
    };
    return new BulletView(&_model, position() + offset[_model.dir], speed[_model.dir]);
}

bool TankView::moving() const {
    return _model.move.x > 0 or _model.move.y > 0;
}

void TankView::explosion() {
    auto widget = New<BigExplosionView>();
    auto animate = widget->to<BigExplosionView>();
    animate->setAnchor(0.5f, 0.5f);
    animate->setPosition(position() + size() * 0.5f);
    animate->play(std::bind(&Widget::removeFromParent, animate));
    _battlefield->addToBottom(widget);
    widget->performLayout();

    auto sound = res::soundName("explosion_1");
    _game.audio().loadEffect(sound);
    _game.audio().playEffect(sound);
}

void TankView::show_score() {
    std::string score[5] = {
            "points_100",
            "points_200",
            "points_300",
            "points_400",
            "points_500",
    };
    auto widget = New<ImageWidget>(res::load_texture(_game.renderer(), res::imageName(score[_model.tier])));
    widget->setSize(this->size());
    widget->setPosition(this->position());
    widget->defer(widget.get(), [](Widget* sender){
        sender->removeFromParent();
    }, 1.0f);
    _battlefield->addToBottom(widget);
    widget->performLayout();
}

void TankView::setBattleField(BattleFieldInterface* battlefield) {
    _battlefield = battlefield;
}

void TankView::onChangeDir(Direction dir) {
    int half_size = Tile::SIZE >> 1;
    //坦克的坐标要卡在1/2大小的图块位置
    if (dir == Direction::LEFT or dir == Direction::RIGHT) {
        auto y = position().y / half_size;
        if ((y-int(y))*10 <= 5) {
            setPositionY(int(y) * half_size);
        } else {
            setPositionY(int(y+1) * half_size);
        }
    } else if (dir == Direction::UP or dir == Direction::DOWN) {
        auto x = position().x / half_size;
        if ((x-int(x))*10 <= 5) {
            setPositionX(int(x) * half_size);
        } else {
            setPositionX(int(x+1) * half_size);
        }
    }
}

void TankView::onUpdate(float delta) {
    _behavior->tick(delta);
}

void TankView::onModifyPosition(Vector2f const& position) {
    _model.position = position;
}

void TankView::onModifySize(Vector2f const& size) {
    _model.bounds.w = Tile::SIZE;
    _model.bounds.h = Tile::SIZE;
}

void TankView::updateMoveSpeed() {
    if (_model.dir == Tank::Direction::MAX) {
        return;
    }
    auto& attr = Tank::getAttribute(_model.party, _model.tier);
    float moveSpeed = attr.moveSpeed;
    Vector2f speed[4] = {
            {0.0f, -moveSpeed},
            {moveSpeed, 0.0f},
            {0.0f, moveSpeed},
            {-moveSpeed, 0.0f},
    };
    _model.move = speed[_model.dir];
}

//=====================================================================================

TileBuilder::TileBuilder(WorldModel* world):_world(world) {

}

void TileBuilder::gen(Array& r, AddTileList const& list) {
    typedef Tile::Type Type;
    for (auto& tile : list) {
        switch (tile.type) {
            case Type::BASE:
                gen(r, "base", {tile.x, tile.y});
                break;
            case Type::BRICK:
                gen(r, "brick", {tile.x, tile.y});
                break;
            case Type::STEEL:
                gen(r, "steel", {tile.x, tile.y});
                break;
            case Type::WATERS:
                gen(r, "water", {tile.x, tile.y});
                break;
            case Type::TREES:
                gen(r, "trees", {tile.x, tile.y});
                break;
            case Type::ICE_FLOOR:
                gen(r, "ice-floor", {tile.x, tile.y});
                break;
            default:
                break;
        }
    }
}

void TileBuilder::gen(Array& result, std::string const& type, Vector2i const& position) {
    if (type == "water") {
        gen_tile(result, TileType::WATER, position);
    } else if (type == "trees") {
        gen_tile(result, TileType::TREES, position);
    } else if (type == "ice-floor") {
        gen_tile(result, TileType::ICE_FLOOR, position);
    } else if (type == "base") {
        gen_tile(result, TileType::BASE, position);
    } else if (type == "brick") {
        get_block(result, TileType::BRICK_0, position);
    } else if (type == "steel") {
        get_block(result, TileType::STEEL_0, position);
    }
}

void TileBuilder::gen_tile(Array& r, TileType t, Vector2i const& position) {
    auto widget = Widget::New<TileView>(t);
    widget->setPosition(position.to<float>());
    widget->to<TileView>()->insert_to(_world);
    r.push_back(widget);
}

void TileBuilder::get_block(Array& r, TileType begin, Vector2i const& position) {
    static int const block_size = Tile::SIZE >> 1;
    Vector2i offset[4] = {
            {0, 0},
            {block_size >> 1, 0},
            {0, block_size >> 1},
            {block_size >> 1, block_size >> 1},
    };
    for (int i = 0; i < 4; ++i) {
        gen_tile(r, TileType(begin + i), position + offset[i]);
    }
}

//=====================================================================================

BulletView::BulletView(TankModel const* tank, Vector2f const& position, Vector2f const& move):
ImageWidget(load_texture(get_dir(move))) {
    _model.id = ++_objectCount;
    _model.sender_id = tank->id;
    _model.position = position - size() * 0.5f;
    _model.move = move;
    _model.party = tank->party;
    _model.add_observer(this);
    this->setPosition(_model.position);
    this->setSize(ImageWidget::size());
    this->enableUpdate(true);
}

void BulletView::insert_to(WorldModel* world) {
    world->bullets.push_back(&_model);
    auto move_behavior = Behavior::Ptr(new BulletMoveBehavior(&_model));
    auto world_collision = Behavior::Ptr(new BulletWorldCollisionBehavior(&_model, world));
    auto tile_collision = Behavior::Ptr(new BulletTileCollisionBehavior(&_model, world));
    auto tank_collision = Behavior::Ptr(new BulletTankCollisionBehavior(&_model, world));
    _behavior = Behavior::Ptr(new SequenceBehavior({move_behavior, world_collision, tile_collision, tank_collision}));
}

void BulletView::play_explosion() {
    auto view = Widget::New<BulletExplosionView>();
    view->setAnchor(0.5f, 0.5f);
    view->setPosition(_model.bounds.x + (_model.bounds.w >> 1), _model.bounds.y + (_model.bounds.h >> 1));
    view->to<BulletExplosionView>()->play();
    parent()->addChild(view);
    view->performLayout();
}

Tank::Direction BulletView::get_dir(Vector2f const& move) const {
    if (move.y > 0) {
        return Tank::Direction::DOWN;
    } else if (move.y < 0) {
        return Tank::Direction::UP;
    } else if (move.x > 0) {
        return Tank::Direction::RIGHT;
    } else if (move.x < 0) {
        return Tank::Direction::LEFT;
    }
    return Tank::Direction::MAX;
}

BulletView::TexturePtr BulletView::load_texture(Tank::Direction dir) const {
    if (dir == Tank::Direction::MAX) {
        return nullptr;
    }
    std::string file[4] = {
            "bullet_up",
            "bullet_right",
            "bullet_down",
            "bullet_left",
    };
    return res::load_texture(_game.renderer(), res::imageName(file[dir]));
}

void BulletView::onUpdate(float delta) {
    _behavior->tick(delta);
}

void  BulletView::onModifyPosition(Vector2f const& position) {
    _model.bounds.x = position.x;
    _model.bounds.y = position.y;
}

void  BulletView::onModifySize(Vector2f const& size) {
    _model.bounds.w = size.x;
    _model.bounds.h = size.y;
}
