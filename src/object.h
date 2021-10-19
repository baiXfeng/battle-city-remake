//
// Created by baifeng on 2021/10/9.
//

#ifndef SDL2_UI_OBJECT_H
#define SDL2_UI_OBJECT_H

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
    TileModel const& model() const;
    void insert_to(WorldModel* world);
    int layer() const;
private:
    void update(float delta) override;
    void draw(SDL_Renderer* renderer) override;
    void onModifyPosition(Vector2f const& position) override;
    void onModifySize(Vector2f const& size) override;
private:
    TYPE _type;
    TileModel _model;
};

class Behavior;
class TankView : public FrameAnimationWidget, public BattleFieldHolder {
public:
    typedef Tank::Direction Direction;
    typedef Tank::Controller Controller;
    typedef std::vector<TexturePtr> Textures;
    typedef std::vector<Textures> TexturesArray;
    typedef std::shared_ptr<Behavior> BehaviorPtr;
public:
    TankView(Tank::Party party, Tank::Tier tier, Tank::Direction dir, bool has_drop = false, Controller c = Tank::AI);
    void setSkin(Controller c, Tank::Tier tier);
    void setSkin(Tank::Tier tier, bool has_drop);
    void setTopEnemySkin();
    void move(Direction dir);
    void turn(Direction dir);
    void stop(Direction dir = Direction::MAX);
    void insert_to(WorldModel* world);
    void fire();
    void onFire();
    bool moving() const;
    void explosion();
    void show_score();
    void modify_shield();
    void open_shield(float duration);
    TankModel const* model() const;
private:
    void onChangeDir(Direction dir);
    void onUpdate(float delta) override;
    void onModifyPosition(Vector2f const& position) override;
    void onModifySize(Vector2f const& size) override;
    void updateMoveSpeed();
    BulletView* createBullet() const;
private:
    bool _force_move;
    TexturesArray _texArr;
    TankModel _model;
    BehaviorPtr _behavior;
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

#endif //SDL2_UI_OBJECT_H
