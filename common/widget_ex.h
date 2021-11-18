//
// Created by baifeng on 2021/11/17.
//

#ifndef SDL2_UI_WIDGET_EX_H
#define SDL2_UI_WIDGET_EX_H

#include "widget.h"

mge_begin

class CellWidget : public Widget {
public:
    CellWidget():_index(0) {}
    inline void setCellIndex(size_t index) {
        _index = index;
    }
    inline size_t getCellIndex() const {
        return _index;
    }
protected:
    size_t _index;
};

class TableWidget;
class WidgetDataSource {
public:
    virtual ~WidgetDataSource() {}
public:
    virtual size_t numberOfCellsInWidget(TableWidget* sender) = 0;
    virtual Vector2i cellSizeForIndex(TableWidget* sender, size_t index) = 0;
    virtual Widget::Ptr cellWidgetAtIndex(TableWidget* sender, size_t index) = 0;
};

class TableWidget : public WindowWidget {
public:
    enum Direction {
        Horizontal = 0,
        Vertical,
    };
    enum MoveDirection {
        MOVE_NEXT = 0,
        MOVE_PREV,
    };
public:
    TableWidget();
public:
    void setDirection(Direction dir);
    Direction getDirection() const;
    void setDataSource(WidgetDataSource* data_source);
    WidgetDataSource* getDataSource() const;
public:
    size_t getCursorIndex() const;
    void startMoveCursor(MoveDirection dir, bool animate = true);
    void stopMoveCursor();
    void reload_data();
    Widget::Ptr dequeueCell();
protected:
    enum CellPosition {
        HEAD = 0,
        TAIL,
    };
    void moveCursorNext(bool animate = true, float duration = 0.12f);
    void moveCursorPrev(bool animate = true, float duration = 0.12f);
    void insertCellAt(CellPosition p);
    void removeCellAt(CellPosition p);
    void onUpdate(float delta) override;
    void checkCells();
protected:
    typedef std::list<Widget::Ptr> CellQueue;
    typedef std::vector<RectI> Rects;
    bool _scrolling;
    int _selectIndex;
    Direction _direction;
    WidgetDataSource* _dataSource;
    WindowWidget* _container;
    Widget* _cursor;
    Rects _rectList;
    CellQueue _idleCells;
    CellQueue _busyCells;
    Vector2f _prevPosition;
};

mge_end

#endif //SDL2_UI_WIDGET_EX_H
