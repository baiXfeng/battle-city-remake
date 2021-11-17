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

class WidgetDataSource {
public:
    virtual ~WidgetDataSource() {}
public:
    virtual size_t numberOfCellsInWidget(Widget* sender) = 0;
    virtual Vector2i cellSizeForIndex(Widget* sender, size_t index) = 0;
    virtual CellWidget* cellWidgetAtIndex(Widget* sender, size_t index) = 0;
};

class TableWidget : public WindowWidget {
public:
    enum Direction {
        Horizontal = 0,
        Vertical,
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
    void moveCursorNext(bool animate = true);
    void moveCursorPrev(bool animate = true);
public:
    void reload_data();
protected:
    Ptr dequeueCell();
    CellWidget* cellAtIndex(size_t index) const;
    void setCursorIndex(size_t index);
    bool isCursorIndexInBox(size_t index, CellWidget* cell = nullptr) const;
protected:
    typedef std::list<Widget::Ptr> CellQueue;
    bool _scrolling;
    int _selectIndex;
    Direction _dir;
    WidgetDataSource* _dataSource;
    WindowWidget* _container;
    Widget* _cursor;
    CellQueue _idleCells;
    CellQueue _busyCells;
};

mge_end

#endif //SDL2_UI_WIDGET_EX_H
