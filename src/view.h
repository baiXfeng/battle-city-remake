//
// Created by baifeng on 2021/9/29.
//

#ifndef SDL2_UI_VIEW_H
#define SDL2_UI_VIEW_H

#include "common/widget.h"
#include "common/quadtree.h"

Widget::Ptr firstScene();

class MaskWidget;
class LogoView : public GamePadWidget {
    typedef std::function<void()> Callback;
public:
    LogoView();
    void setFinishCall(Callback const& cb);
private:
    void onFadeIn(float v);
    void onFadeOut(float v);
    void onEnter() override;
    void onButtonDown(int key) override;
private:
    bool _canClick;
    MaskWidget* _mask;
    Callback _callback;
};

class StartView : public GamePadWidget {
public:
    StartView();
private:
    void onEnter() override;
    void onButtonDown(int key) override;
    void onStart(int index);
private:
    bool _canSelect;
    int _index;
    std::vector<Vector2f> _position;
};

class SelectLevelView : public GamePadWidget {
public:
    SelectLevelView();
private:
    void onButtonDown(int key) override;
    void onButtonUp(int key) override;
    void addLevel();
    void subLevel();
    void autoAddLevel(bool add);
    void stopAutoAddLevel();
private:
    int _level;
    TTFLabel* _label;
    CurtainWidget* _curtain;
    float _duration;
};

class BattleView : public WindowWidget {
public:
    BattleView();
};

class TankView;
class BattleFieldView : public GamePadWidget {
    typedef DebugQuadTree<Widget::Ptr> DebugQuadTreeT;
public:
    BattleFieldView();
private:
    void onUpdate(float delta) override;
    void draw(SDL_Renderer* renderer) override;
    void onDraw(SDL_Renderer* renderer) override;
    void onButtonDown(int key) override;
    void onButtonUp(int key) override;
    void addElement(Widget::Ptr& widget);
    void sortElements();
    void procTankControl();
    void gameOver();
private:
    void onTankUpdateQuadTree(Widget::Ptr const& tank);
    void onTankMoveCollision(TankView* tank);
private:
    void add_key(int key);
    bool remove_key(int key);
private:
    typedef DebugQuadTreeT WidgetQuadTree;
    typedef std::shared_ptr<WidgetQuadTree> QuadTreePtr;
    Widget* _root;
    TankView* _player;
    QuadTreePtr _quadtree;
    std::list<int> _keylist;
    WidgetQuadTree::SquareList _checklist;
};

class BattleInfoView : public Widget {
public:
    BattleInfoView();
    ~BattleInfoView();
private:
    void onEvent(Event const& e) override;
    void onEnemyNumberChanged(int n);
    void onPlayerNumberChanged(int n);
private:
    ImageWidget* createEnemyIcon();
};

class TileData;
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
    typedef std::shared_ptr<TileData> TileDataPtr;
public:
    TileView(TYPE t);
public:
    void setType(TYPE t);
    int type() const;
private:
    void update(float delta) override;
    void draw(SDL_Renderer* renderer) override;
    void onDirty() override;
private:
    TYPE _type;
    TileDataPtr _data;
};

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
    typedef std::shared_ptr<TileData> TileDataPtr;
public:
    TankView(TYPE t, TexturesArray const& array);
    void move(Direction dir);
    void turn(Direction dir);
    void stop(Direction dir = MAX);
private:
    void onUpdate(float delta) override;
    void onDirty() override;
    void limitPosition();
private:
    TYPE _type;
    Direction _dir;
    Vector2f _move;
    TexturesArray _texArr;
    TileDataPtr _data;
};

class TileBuilder {
public:
    typedef std::vector<Widget::Ptr> TileArray;
    typedef TileView::TYPE TileType;
public:
    void gen(TileArray& r, std::string const& type, Vector2f const& position);
private:
    void gen_tile(TileArray& r, TileType t, Vector2f const& position);
    void get_block(TileArray& r, TileType begin, Vector2f const& position);
};

class TankBuilder {
public:
    typedef std::vector<Widget::Ptr> TankArray;
    typedef TankView::Direction Direction;
    typedef TankView::TYPE TankType;
    typedef TankView::TexturesArray TexturesArray;
public:
    void gen(TankArray& r, TankType t, Vector2f const& position);
private:
    void gen_textures(TexturesArray& array, TankType t);
};

class ScoreView : public WindowWidget {
public:
    ScoreView();
};

class GameOverView : public GamePadWidget {
public:
    GameOverView();
private:
    void onButtonDown(int key) override;
};

#endif //SDL2_UI_VIEW_H
