//
// Created by baifeng on 2021/9/29.
//

#ifndef SDL2_UI_VIEW_H
#define SDL2_UI_VIEW_H

#include "common/widget.h"

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

class BattleView : public GamePadWidget {
public:
    BattleView();
};

class BattleFieldView : public WindowWidget {
public:
    BattleFieldView();
private:
    void addElement(Widget::Ptr& widget);
    void sortElements();
private:
    Widget* _root;
};

class BattleInfoView : public Widget {
public:
    BattleInfoView();
private:
    ImageWidget* createEnemyIcon();
};

class TileData;
class TileView : public ImageWidget {
public:
    enum TYPE {
        NONE = 0,
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
    TYPE type() const;
private:
    void update(float delta) override;
    void draw(SDL_Renderer* renderer) override;
private:
    TYPE _type;
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

#endif //SDL2_UI_VIEW_H
