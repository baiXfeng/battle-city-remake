//
// Created by baifeng on 2021/11/19.
//

#ifndef SDL2_UI_GRIDMAP_H
#define SDL2_UI_GRIDMAP_H

#include "widget.h"

mge_begin

class TileWidget : public Widget {
public:
    TileWidget() {}
    inline void setTileIndex(size_t index) {
        _index = index;
    }
    inline size_t getTileIndex() const {
        return _index;
    }
protected:
    size_t _index;
};

class GridMapWidget;
class GridMapDataSource {
public:
    virtual ~GridMapDataSource() {}
public:
    virtual size_t numberOfLayersInWidget(GridMapWidget* sender) = 0;
    virtual Vector2i sizeOfGridMap(GridMapWidget* sender) = 0;
    virtual Vector2i sizeOfGridTile(GridMapWidget* sender) = 0;
    virtual Widget::Ptr tileWidgetAtPosition(GridMapWidget* sender, int layerIndex, Vector2i const& position) = 0;
};

class GridMapWidget;
class GridMapLayer : public WindowWidget {
    friend class GridMapWidget;
public:
    typedef std::list<Widget::Ptr> TileQueue;
public:
    GridMapLayer(GridMapWidget* gridmap, int layerIndex);
protected:
    Widget::Ptr dequeueTile();
    void enqueueTile(Widget::Ptr const& widget);
    void tagTile(size_t tile_index, TileWidget* tile_view);
    void makeTile(Vector2i const& tile_pos);
    void checkTiles();
protected:
    void removeTop();
    void removeBottom();
    void removeLeft();
    void removeRight();
    void insertTop();
    void insertBottom();
    void insertLeft();
    void insertRight();
protected:
    typedef std::unordered_map<int, TileWidget*> TilePool;
    int _layerIndex;
    int _minIndex, _maxIndex;
    GridMapWidget* _gridmap;
    TileQueue _idleTiles;
    TileQueue _busyTiles;
    TilePool _tiles;
};

class GridMapCamera : public WindowWidget {
public:
    enum EVENT {
        DID_SCROLL = 0xABEF2001,
    };
    enum MoveDirection {
        MOVE_UP = 0,
        MOVE_RIGHT,
        MOVE_DOWN,
        MOVE_LEFT,
    };
    typedef std::vector<MoveDirection> MoveDirs;
    GridMapCamera(Widget* container);
public:
    void move(Vector2f const& speed);
    MoveDirs const& move_dirs() const;
    bool inCamera(RectI const& r) const;
protected:
    void onUpdate(float delta) override;
protected:
    Widget* _container;
    Vector2f _speed;
    MoveDirs _moveDirs;
};

class GridMapWidget : public WindowWidget {
public:
    typedef GridMapDataSource DataSource;
public:
    GridMapWidget();
public:
    void setDataSource(DataSource* data_source);
    DataSource* getDataSource() const;
    GridMapLayer* getLayer(int index) const;
    GridMapCamera* getCamera() const;
public:
    void reload_data();
    Widget::Ptr dequeueTile(int layer);
protected:
    void checkTiles(Widget* sender);
    void onModifySize(Vector2f const& size) override;
protected:
    typedef std::vector<GridMapLayer*> GridLayers;
    DataSource* _dataSource;
    WindowWidget* _container;
    GridMapCamera* _camera;
    GridLayers _tileLayers;
};

mge_end

#endif //SDL2_UI_GRIDMAP_H
