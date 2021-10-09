//
// Created by baifeng on 2021/10/9.
//

#ifndef SDL2_UI_OBJECT_H
#define SDL2_UI_OBJECT_H

#include "common/widget.h"
#include "data.h"

class TileView : public ImageWidget {
public:
    enum TYPE {
        NONE = 0x100,
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
class TankView : public FrameAnimationWidget {
public:
    enum Direction {
        UP = 0,
        RIGHT,
        DOWN,
        LEFT,
        MAX,
    };
    enum TYPE {
        NONE = 0x200,
        PLAYER_1,
        PLAYER_2,
        PLAYER_3,
        PLAYER_4,
    };
    typedef std::vector<TexturePtr> Textures;
    typedef std::vector<Textures> TexturesArray;
    typedef std::shared_ptr<Behavior> BehaviorPtr;
public:
    TankView(TYPE t, TexturesArray const& array);
    void move(Direction dir);
    void turn(Direction dir);
    void stop(Direction dir = MAX);
    void insert_to(WorldModel* world);
private:
    void onChangeDir(Direction dir);
    void onUpdate(float delta) override;
    void onDirty() override;
    void onModifyPosition(Vector2f const& position) override;
private:
    TYPE _type;
    Direction _dir;
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

class TankBuilder {
public:
    typedef std::vector<Widget::Ptr> Array;
    typedef TankView::Direction Direction;
    typedef TankView::TYPE TankType;
    typedef TankView::TexturesArray TexturesArray;
public:
    TankBuilder(WorldModel* world);
    void gen(Array& r, TankType t, Vector2f const& position);
private:
    void gen_textures(TexturesArray& array, TankType t);
private:
    WorldModel* _world;
};

#endif //SDL2_UI_OBJECT_H
