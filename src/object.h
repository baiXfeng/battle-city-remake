//
// Created by baifeng on 2021/10/9.
//

#ifndef BATTLE_CITY_OBJECT_H
#define BATTLE_CITY_OBJECT_H

#include "common/widget.h"
#include "data.h"
#include "view.h"

class BattleFieldHolder {
public:
    typedef BattleFieldInterface BattleField;
public:
    BattleFieldHolder():_battlefield(nullptr) {}
    virtual ~BattleFieldHolder() {}
public:
    void setBattleField(BattleField* battlefield) {
        _battlefield = battlefield;
    }
    BattleField* battleField() const {
        return _battlefield;
    }
protected:
    BattleField* _battlefield;
};

class TileView : public ImageWidget {
public:
    enum TYPE {
        TYPE_BEGIN = 0x100,
        BASE,
        BRICK_0,
        BRICK_1,
        BRICK_2,
        BRICK_3,
        STEEL_0,
        STEEL_1,
        STEEL_2,
        STEEL_3,
        WATER,
        TREES,
        ICE_FLOOR,
    };
public:
    TileView(TYPE t);
public:
    void setType(TYPE t);
    int type() const;
    TileModel const* model() const;
    void insert_to(WorldModel* world);
    int layer() const;
private:
    void update(float delta) override;
    void draw(SDL_Renderer* renderer) override;
    void onModifyPosition(Vector2f const& position) override;
    void onModifySize(Vector2f const& size) override;
    void onVisible(bool visible) override;
private:
    TYPE _type;
    TileModel _model;
};

class TankAnimate : public RenderCopy {
public:
    typedef std::shared_ptr<Texture> TexturePtr;
    typedef std::vector<TexturePtr> Textures;
    typedef std::vector<Textures> TexturesArray;
    typedef std::shared_ptr<TankAnimate> Ptr;
public:
    TankAnimate(TankModel const* model): _model(model) {}
    virtual void update(float delta) {}
protected:
    TankModel const* _model;
};

class EnemyTankAnimate : public TankAnimate {
public:
    EnemyTankAnimate(TankModel* model);
protected:
    void update(float delta) override;
    void draw(SDL_Renderer* renderer, Vector2i const& position = {0, 0}) override;
protected:
    TankAnimate::Ptr _animate[2];
};

// Tank: A, B, C
class ABC_EnemyTankAnimate : public TankAnimate {
public:
    ABC_EnemyTankAnimate(TankModel* model);
    void update(float delta) override;
protected:
    int _frameIndex;
    float _frameTicks;
    float const _maxFrameTicks;
    TexturesArray _animates;
};

// Tank: D
class D_EnemyTankAnimate : public TankAnimate {
public:
    D_EnemyTankAnimate(TankModel* model);
protected:
    void update(float delta) override;
    void tierC_update(float delta);
    void modifySkin();
protected:
    int _hp;
    int _switchIndex;
    int _frameIndex;
    float _frameTicks;
    float const _maxFrameTicks;
    TexturesArray _animates;
    TexturesArray _grayAnimates;
};

// Player
class PlayerTankAnimate : public TankAnimate {
public:
    PlayerTankAnimate(TankModel* model);
    void update(float delta) override;
protected:
    int _tier;
    int _frameIndex;
    float _frameTicks;
    float const _maxFrameTicks;
    TexturesArray _animates;
};

class Behavior;
class TankView : public Widget, public BattleFieldHolder {
public:
    typedef Tank::Direction Direction;
    typedef Tank::Controller Controller;
    typedef std::shared_ptr<Behavior> BehaviorPtr;
    typedef std::shared_ptr<TankAnimate> TankAnimatePtr;
public:
    TankView(Tank::Party party, Tank::Tier tier, Tank::Direction dir, bool has_drop = false, Controller c = Tank::AI);
    void move(Direction dir);
    void turn(Direction dir);
    void stop(Direction dir = Direction::MAX);
    void insert_to(WorldModel* world);
    void fire();
    void createBullet();
    void explosion();
    void show_score();
    void modify_shield();
    void open_shield(float duration);
    TankModel const* model() const;
private:
    void onChangeDir(Direction dir);
    void onUpdate(float delta) override;
    void onDraw(SDL_Renderer* renderer) override;
    void onModifyPosition(Vector2f const& position) override;
    void onModifySize(Vector2f const& size) override;
    void onVisible(bool visible) override;
    void updateMoveSpeed();
private:
    TankModel _model;
    BehaviorPtr _behavior;
    TankAnimatePtr _tankAnimate[2];
};

class TileBuilder {
public:
    typedef std::vector<Widget::Ptr> Array;
    typedef TileView::TYPE TileType;
public:
    TileBuilder(WorldModel* world);
    void gen(Array& r, AddTileList const& list);
private:
    void gen(Array& r, std::string const& type, Vector2i const& position);
    void gen_tile(Array& r, TileType t, Vector2i const& position);
    void get_block(Array& r, TileType begin, Vector2i const& position);
private:
    WorldModel* _world;
};

class BulletView : public ImageWidget, public BattleFieldHolder {
public:
    typedef std::shared_ptr<Behavior> BehaviorPtr;
public:
    BulletView(TankModel const* tank, Vector2f const& position, Vector2f const& move);
    void insert_to(WorldModel* world);
    void play_explosion();
private:
    Tank::Direction get_dir(Vector2f const& move) const;
    TexturePtr load_texture(Tank::Direction dir) const;
    void onUpdate(float delta) override;
    void onModifyPosition(Vector2f const& position) override;
    void onModifySize(Vector2f const& size) override;
private:
    BulletModel _model;
    BehaviorPtr _behavior;
};

class PropView : public ImageWidget, public BattleFieldHolder {
public:
    typedef std::shared_ptr<Behavior> BehaviorPtr;
public:
    PropView(Tank::PowerUp type);
public:
    void insert_to(WorldModel* world);
    void show_score();
    PropModel const* model() const;
private:
    void onModifyPosition(Vector2f const& position) override;
    void onModifySize(Vector2f const& size) override;
    void onUpdate(float delta) override;
private:
    PropModel _model;
    BehaviorPtr _behavior;
};

#endif //BATTLE_CITY_OBJECT_H
