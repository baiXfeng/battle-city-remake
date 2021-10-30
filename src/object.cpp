//
// Created by baifeng on 2021/10/9.
//

#include "object.h"
#include "common/action.h"
#include "common/loadres.h"
#include "common/game.h"
#include "common/audio.h"
#include "behaviors.h"
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

//=====================================================================================

TileView::TileView(TYPE t):
ImageWidget(nullptr),
_type(TYPE_BEGIN) {
    _model.id = ++_objectCount;
    _model.layer = 0;
    _model.visible = true;
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
            texture = res::load_texture(_game.renderer(), res::imageName("base"));
            setTexture(texture);
            _model.type = Tile::BASE;
            break;
        case BRICK_0:
        case BRICK_1:
        case BRICK_2:
        case BRICK_3:
        {
            texture = res::load_texture(_game.renderer(), res::imageName("wall_brick"));
            int half_width = Tile::SIZE >> 2;
            int half_height = Tile::SIZE >> 2;
            SDL_Rect srcrect[4] = {
                    {0, 0, half_width, half_height},
                    {half_width, 0, half_width, half_height},
                    {0, half_height, half_width, half_height},
                    {half_width, half_height, half_width, half_height},
            };
            setTexture(texture, srcrect[t-BRICK_0]);
            _model.type = Tile::BRICK;
            _model.bounds.w = half_width;
            _model.bounds.h = half_height;
            return;
        }
            break;
        case STEEL_0:
        case STEEL_1:
        case STEEL_2:
        case STEEL_3:
        {
            texture = res::load_texture(_game.renderer(), res::imageName("wall_steel"));
            int half_width = Tile::SIZE >> 2;
            int half_height = Tile::SIZE >> 2;
            SDL_Rect srcrect[4] = {
                    {0, 0, half_width, half_height},
                    {half_width, 0, half_width, half_height},
                    {0, half_height, half_width, half_height},
                    {half_width, half_height, half_width, half_height},
            };
            setTexture(texture, srcrect[t-STEEL_0]);
            _model.type = Tile::STEEL;
            _model.bounds.w = half_width;
            _model.bounds.h = half_height;
            return;
        }
            break;
        case TREES:
            texture = res::load_texture(_game.renderer(), res::imageName("trees"));
            setTexture(texture);
            _model.layer = 2;
            _model.type = Tile::TREES;
            break;
        case ICE_FLOOR:
            texture = res::load_texture(_game.renderer(), res::imageName("ice_floor"));
            setTexture(texture);
            _model.layer = 0;
            _model.type = Tile::ICE_FLOOR;
            break;
        case WATER:
        {
            FrameAnimationAction::Frames frames = {
                    res::load_texture(_game.renderer(), res::imageName("water_1")),
                    res::load_texture(_game.renderer(), res::imageName("water_2")),
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

TileModel const* TileView::model() const {
    return &_model;
}

void TileView::insert_to(WorldModel* world) {
    world->tiles.insert(&_model);
}

int TileView::layer() const {
    return _model.layer;
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

void TileView::onVisible(bool visible) {
    _model.visible = visible;
}

//=====================================================================================

EnemyTankAnimate::EnemyTankAnimate(TankModel* model):TankAnimate(model) {
    _animate[0] = Ptr(new ABC_EnemyTankAnimate(model));
    _animate[1] = Ptr(new D_EnemyTankAnimate(model));
}

void EnemyTankAnimate::update(float delta) {
    auto& animate = _animate[ _model->tier == Tank::D ? 1 : 0 ];
    animate->update(delta);
    setSize(animate->size());
}

void EnemyTankAnimate::draw(SDL_Renderer* renderer, Vector2i const& position) {
    auto& animate = _animate[ _model->tier == Tank::D ? 1 : 0 ];
    animate->draw(renderer, position);
}

//=====================================================================================

ABC_EnemyTankAnimate::ABC_EnemyTankAnimate(TankModel* model):
TankAnimate(model),
_frameIndex(0),
_frameTicks(0.0f),
_maxFrameTicks(0.06f) {
    _animates = skin::getEnemySkin(_model->tier, _model->has_drop);
    _frameTicks = _maxFrameTicks;
}

void ABC_EnemyTankAnimate::update(float delta) {
    if (_model->dir == Tank::Direction::MAX) {
        return;
    }
    if ((_frameTicks += delta) >= _maxFrameTicks) {
        auto& frames = _animates[_model->dir];
        _frameTicks -= _maxFrameTicks;
        _frameIndex = ++_frameIndex >= frames.size() ? 0 : _frameIndex;
        setTexture(frames[_frameIndex]->data());
    }
}

//=====================================================================================

D_EnemyTankAnimate::D_EnemyTankAnimate(TankModel* model):
TankAnimate(model),
_hp(0),
_switchIndex(0),
_frameIndex(0),
_frameTicks(0.0f),
_maxFrameTicks(0.06f) {
    if (model->has_drop) {
        _animates = skin::getEnemySkin(_model->tier, _model->has_drop);
    } else {
        _animates = skin::getDEnemySkin(_model->tier);
    }
    _grayAnimates = skin::getDEnemySkin(Tank::A);
    _hp = model->hp;
    _frameTicks = _maxFrameTicks;
}

void D_EnemyTankAnimate::update(float delta) {
    if (_model->dir == Tank::Direction::MAX) {
        return;
    }
    if (_hp != _model->hp) {
        _hp = _model->hp;
        modifySkin();
    }
    if (_hp == 3) {
        tierC_update(delta);
        return;
    }
    if ((_frameTicks += delta) >= _maxFrameTicks) {
        auto& frames = _animates[_model->dir];
        _frameTicks -= _maxFrameTicks;
        _frameIndex = ++_frameIndex >= frames.size() ? 0 : _frameIndex;
        setTexture(frames[_frameIndex]->data());
    }
}

void D_EnemyTankAnimate::tierC_update(float delta) {
    auto& frames = _animates[_model->dir];
    auto& grays = _grayAnimates[_model->dir];
    Texture::Ptr texture[2] = {
            frames[ _frameIndex ],
            grays[ _frameIndex ],
    };
    setTexture(texture[ _switchIndex = 1 - _switchIndex ]->data());
    if ((_frameTicks += delta) >= _maxFrameTicks) {
        _frameTicks -= _maxFrameTicks;
        _frameIndex = ++_frameIndex >= frames.size() ? 0 : _frameIndex;
    }
}

void D_EnemyTankAnimate::modifySkin() {
    auto tier = Tank::Tier(_hp-1);
    _animates = skin::getDEnemySkin(tier);
}

//=====================================================================================

PlayerTankAnimate::PlayerTankAnimate(TankModel* model):
TankAnimate(model),
_frameIndex(0),
_frameTicks(0.0f),
_maxFrameTicks(0.06f) {
    _animates = skin::getPlayerSkin(_model->tier, _model->controller);
    _tier = _model->tier;
    _frameTicks = _maxFrameTicks;
}

void PlayerTankAnimate::update(float delta) {
    if (_model->dir == Tank::Direction::MAX) {
        return;
    }
    if (_tier != _model->tier) {
        _tier = _model->tier;
        _animates = skin::getPlayerSkin(_model->tier, _model->controller);
        _tier = _model->tier;
        _frameTicks = _maxFrameTicks;
    }
    if ((_frameTicks += delta) >= _maxFrameTicks) {
        auto& frames = _animates[_model->dir];
        if (not _model->moving) {
            setTexture(frames[0]->data());
            return;
        }
        _frameTicks -= _maxFrameTicks;
        _frameIndex = ++_frameIndex >= frames.size() ? 0 : _frameIndex;
        setTexture(frames[_frameIndex]->data());
    }
}

//=====================================================================================

std::string shot_sound = res::soundName("bullet_shot");

TankView::TankView(Tank::Party party, Tank::Tier tier, Tank::Direction dir, bool has_drop, Controller c) {
    _model.id = ++_objectCount;
    _model.fire = false;
    _model.shield = false;
    _model.visible = true;
    _model.has_drop = has_drop;
    _model.dir = dir;
    _model.party = party;
    _model.tier = tier;
    _model.controller = c;
    _model.size = {Tile::SIZE, Tile::SIZE};
    _model.dir = Direction::MAX;
    _model.add_observer(this);

    auto& attr = Tank::getAttribute(party, tier);
    _model.hp = attr.health;

    enableUpdate(true);
    _game.audio().loadEffect(shot_sound);

    _tankAnimate[Tank::PLAYER] = TankAnimatePtr(new PlayerTankAnimate(&_model));
    _tankAnimate[Tank::ENEMY] = TankAnimatePtr(new EnemyTankAnimate(&_model));
}

void TankView::move(Direction dir, bool gamepad_controll) {
    if (_model.dir != dir) {
        this->onChangeDir(dir);
    }
    _model.dir = dir;
    _model.moving = true;
    this->updateMoveSpeed();

    if (gamepad_controll) {
        // 玩家操作同时取消冰面滑行动作
        stopAction("TankView::on_ice_floor:action");
    }
}

void TankView::turn(Direction dir) {
    _model.dir = dir;
}

void TankView::stop(Direction dir) {
    if (dir == Direction::UP or dir == Direction::DOWN) {
        _model.move.y = 0.0f;
    } else if (dir == Direction::LEFT or dir == Direction::RIGHT) {
        _model.move.x = 0.0f;
    } else {
        _model.move = {0, 0};
    }
    _model.moving = false;
}

void TankView::insert_to(WorldModel* world) {
    _model.position = position();
    world->tanks.push_back(&_model);
    auto tank_ai = Behavior::Ptr(new TankAI_Behavior(&_model));
    auto tank_fire = Behavior::Ptr(new TankFireBehavior(&_model, &world->bullets));
    auto tank_move = Behavior::Ptr(new TankMoveBehavior(&_model, world->bounds));
    auto tank_collision = Behavior::New<TankCollisionBehavior>(&_model, &world->tanks);
    auto tile_collision = Behavior::New<TankTileCollisionBehavior>(&_model, &world->tiles);
    _behavior = Behavior::Ptr(new SequenceBehavior({tank_ai, tank_fire, tank_move, tank_collision, tile_collision}));
}

void TankView::fire() {
    _model.fire = true;
}

void TankView::createBullet() {

    if (_model.party == Tank::PLAYER) {
        // 只有玩家的坦克才会播放子弹音效
        _game.audio().playEffect(shot_sound);
    }
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
    auto world = &_game.get<WorldModel>("world_model");
    auto bullet = new BulletView(&_model, position() + offset[_model.dir], speed[_model.dir]);
    bullet->setBattleField(battleField());
    bullet->insert_to(world);

    Widget::Ptr widget(bullet);
    battleField()->addToMiddle(widget);
    widget->performLayout();
}

void TankView::explosion() {
    auto widget = New<BigExplosionView>();
    auto animate = widget->to<BigExplosionView>();
    animate->setAnchor(0.5f, 0.5f);
    animate->setPosition(position() + size() * 0.5f);
    animate->play();
    _battlefield->addToMiddle(widget);
    widget->performLayout();
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

void TankView::modify_shield() {
    if (_model.shield) {
        if (find("shield:animate")) {
            return;
        }
        auto widget = New<FrameAnimationWidget>();
        auto animate = widget->to<FrameAnimationWidget>();
        animate->setFrames(skin::getShieldSkin());
        animate->setName("shield:animate");
        animate->play(0.1f);
        addChild(widget);
        widget->performLayout();
    } else {
        auto view = find("shield:animate");
        if (view) {
            view->removeFromParent();
        }
    }
}

void TankView::open_shield(float duration) {
    auto tank = &_model;
    auto delay = Action::New<Delay>(duration);
    auto call = Action::New<CallBackT<TankModel*>>(tank, [](TankModel* tank){
        tank->shield = false;
        tank->modifyShield();
    });
    auto action = Action::Ptr(new Sequence({delay, call}));
    auto scene = _game.screen().scene_back();
    action->setName("player:shield");
    scene->stopAction("player:shield");
    scene->runAction(action);
    tank->shield = true;
    tank->modifyShield();
}

void TankView::on_ice_floor() {
    std::string actionName = "TankView::on_ice_floor:action";
    if (hasAction(actionName)) {
        return;
    }
    auto duration = Tank::getGlobalFloat("PLAYER_ICE_FLOOR_SLIDING_DURATION");
    auto delay = Action::New<Delay>(duration);
    auto callback = Action::New<CallBackVoid>([this]{
        this->stop();
    });
    auto action = Action::Ptr(new Sequence({delay, callback}));
    action->setName(actionName);
    runAction(action);
    this->move(_model.dir, false);
}

TankModel const* TankView::model() const {
    return &_model;
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
    if (not _visible) {
        return;
    }
    _behavior->tick(delta);
    _tankAnimate[_model.party]->update(delta);
    setSize(_tankAnimate[_model.party]->size().to<float>());
}

void TankView::onDraw(SDL_Renderer* renderer) {
    _tankAnimate[_model.party]->draw(renderer, _global_position.to<int>());
}

void TankView::onModifyPosition(Vector2f const& position) {
    _model.position = position;
    _model.bounds.x = position.x;
    _model.bounds.y = position.y;
}

void TankView::onModifySize(Vector2f const& size) {
    _model.bounds.w = Tile::SIZE;
    _model.bounds.h = Tile::SIZE;
}

void TankView::onVisible(bool visible) {
    _model.visible = visible;
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
    auto& attr = Tank::getAttribute(tank->party, tank->tier);
    _model.id = ++_objectCount;
    _model.sender_id = tank->id;
    _model.wall_damage = attr.bulletWallDamage;
    _model.position = position - size() * 0.5f;
    _model.move = move;
    _model.party = tank->party;
    if (_model.party == Tank::PLAYER and tank->tier == Tank::D) {
        // 最高等级玩家坦克可以消除铁块
        _model.destroy_steel = true;
    }
    _model.add_observer(this);
    this->setPosition(_model.position);
    this->setSize(ImageWidget::size());
    this->enableUpdate(true);
}

void BulletView::insert_to(WorldModel* world) {
    world->bullets.push_back(&_model);
    auto move_behavior = Behavior::Ptr(new BulletMoveBehavior(&_model));
    auto world_collision = Behavior::Ptr(new BulletWorldCollisionBehavior(&_model, world));
    auto tank_collision = Behavior::Ptr(new BulletTankCollisionBehavior(&_model, world));
    auto tile_collision = Behavior::Ptr(new BulletTileCollisionBehavior(&_model, world));
    auto bullet_collision = Behavior::Ptr(new BulletBulletCollisionBehavior(&_model, world));
    _behavior = Behavior::Ptr(new SequenceBehavior({
        move_behavior,
        world_collision,
        tank_collision,
        tile_collision,
        bullet_collision,
    }));
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

//=====================================================================================

PropView::PropView(Tank::PowerUp type):ImageWidget(
        res::load_texture(_game.renderer(), res::powerupName(type))
) {
    _model.id = ++_objectCount;
    _model.type = type;
    _model.bounds = {
            0, 0, Tile::SIZE, Tile::SIZE,
    };
    _model.add_observer(this);
    enableUpdate(true);
}

void PropView::insert_to(WorldModel* world) {
    world->props.push_back(&_model);
    _behavior = Behavior::New<PropCollisionBehavior>(&_model, &world->tanks, &world->props);
}

void PropView::show_score() {
    auto widget = New<ImageWidget>(res::load_texture(_game.renderer(), res::imageName("points_500")));
    widget->setSize(this->size());
    widget->setPosition(this->position());
    widget->defer(widget.get(), [](Widget* sender){
        sender->removeFromParent();
    }, 1.0f);
    _battlefield->addToTop(widget);
    widget->performLayout();
}

PropModel const* PropView::model() const {
    return &_model;
}

void PropView::onModifyPosition(Vector2f const& position) {
    _model.bounds.x = position.x;
    _model.bounds.y = position.y;
}

void PropView::onModifySize(Vector2f const& size) {
    _model.bounds.w = size.x;
    _model.bounds.h = size.y;
}

void PropView::onUpdate(float delta) {
    _behavior->tick(delta);
}
