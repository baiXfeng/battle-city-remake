//
// Created by baifeng on 2021/11/17.
//

#include "widget_ex.h"
#include "action.h"

mge_begin

//=====================================================================================

    TableWidget::TableWidget():_dir(Vertical), _dataSource(nullptr), _container(nullptr), _cursor(nullptr), _selectIndex(0), _scrolling(false) {
        addChild(Ptr(_container = new WindowWidget));

        auto mask = new MaskWidget({0, 0, 0, 200});
        addChild(Ptr(_cursor = mask));
    }

    void TableWidget::setDirection(Direction dir) {
        _dir = dir;
    }

    TableWidget::Direction TableWidget::getDirection() const {
        return _dir;
    }

    void TableWidget::setDataSource(WidgetDataSource* data_source) {
        _dataSource = data_source;
    }

    WidgetDataSource* TableWidget::getDataSource() const {
        return _dataSource;
    }

    void TableWidget::setCursorIndex(size_t index) {
        if (index >= _dataSource->numberOfCellsInWidget(this)) {
            return;
        }
        if (_cursor) {
            auto cell = cellAtIndex(index);
            if (isCursorIndexInBox(index, cell)) {
                if (_dir == Vertical) {
                    _cursor->setPositionY(_container->position().y + cell->position().y);
                } else {
                    _cursor->setPositionX(_container->position().x + cell->position().x);
                }
                _cursor->setSize(cell->size());
                _selectIndex = index;
            }
        }
    }

    size_t TableWidget::getCursorIndex() const {
        return _selectIndex;
    }

    bool TableWidget::isCursorIndexInBox(size_t index, CellWidget* cell) const {
        if (cell == nullptr) {
            cell = cellAtIndex(index);
        }
        if (cell) {
            if (_dir == Vertical) {
                auto y = _container->position().y + cell->position().y;
                auto h = cell->size().y;
                if (y < 0 or y + h > size().y) {
                    return false;
                }
            } else {
                auto x = _container->position().x + cell->position().x;
                auto w = cell->size().x;
                if (x < 0 or x + w > size().x) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    TableWidget::Ptr TableWidget::dequeueCell() {
        if (_idleCells.empty()) {
            return nullptr;
        }
        auto r = _idleCells.back();
        _idleCells.pop_back();
        return r;
    }

    CellWidget* TableWidget::cellAtIndex(size_t index) const {
        for (auto& w : _busyCells) {
            auto r = w->to<CellWidget>();
            if (r and r->getCellIndex() == index) {
                return r;
            }
        }
        return nullptr;
    }

    void TableWidget::reload_data() {
        assert(_dataSource != nullptr && "TableWidget::reload_data fail.");

        Vector2f containerSize;
        for (int i = 0; i < _dataSource->numberOfCellsInWidget(this); ++i) {
            auto cellSize = _dataSource->cellSizeForIndex(this, i);
            if (_dir == Vertical) {
                containerSize.y += cellSize.y;
            } else {
                containerSize.x += cellSize.x;
            }
        }
        if (_dir == Vertical) {
            containerSize.x = size().x;
        } else {
            containerSize.y = size().y;
        }
        _selectIndex = 0;
        _container->setSize(containerSize);
        _container->setPosition(0.0f, 0.0f);

        int nextIndex = 0;
        Vector2f position;
        for (nextIndex = 0; nextIndex < _dataSource->numberOfCellsInWidget(this); ++nextIndex) {
            auto cellSize = _dataSource->cellSizeForIndex(this, nextIndex);
            auto cell = this->dequeueCell();
            if (cell == nullptr) {
                _busyCells.push_back(cell = Ptr(_dataSource->cellWidgetAtIndex(this, nextIndex)));
                _container->addChild(cell);
            }
            cell->setVisible(true);
            cell->setSize(cellSize.to<float>());
            cell->setPosition(position);
            cell->to<CellWidget>()->setCellIndex(nextIndex);
            if (_selectIndex == nextIndex) {
                _cursor->setPosition(position);
                _cursor->setSize(cell->size());
            }
            if (_dir == Vertical) {
                position.y += cellSize.y;
                if (_container->position().y + position.y >= this->size().y) {
                    break;
                }
            } else {
                position.x += cellSize.x;
                if (_container->position().x + position.x >= this->size().x) {
                    break;
                }
            }
        }
        if (_idleCells.empty()) {
            auto cell = Ptr(_dataSource->cellWidgetAtIndex(this, ++nextIndex));
            cell->setVisible(false);
            _idleCells.push_back(cell);
            _container->addChild(cell);
        }
    }

    void TableWidget::moveCursorNext(bool animate) {
        if (_selectIndex + 1 >= _dataSource->numberOfCellsInWidget(this) or this->_scrolling) {
            return;
        }
        auto cell = cellAtIndex(_selectIndex+1);
        if (cell == nullptr) {
            return;
        }
        Vector2f offset;
        if (Vertical == _dir) {
            offset.y = cell->size().y;
        } else {
            offset.x = cell->size().x;
        }
        bool in_box = isCursorIndexInBox(_selectIndex+1, cell);
        auto sender = in_box ? _cursor : _container;
        auto moveby = Action::New<MoveBy>(sender, offset * (in_box ? 1 : -1), 0.12f);
        auto call = Action::New<CallBackVoid>([this]{
            this->_scrolling = false;
            this->_selectIndex++;
        });
        auto action = Action::Ptr(new Sequence({moveby, call}));
        action->setName("TableWidget::moveCursor");
        this->runAction(action);
        this->_scrolling = true;
    }

    void TableWidget::moveCursorPrev(bool animate) {
        if (_selectIndex - 1 <= -1 or this->_scrolling) {
            return;
        }
        auto cell = cellAtIndex(_selectIndex-1);
        if (cell == nullptr) {
            return;
        }
        Vector2f offset;
        if (Vertical == _dir) {
            offset.y = -cell->size().y;
        } else {
            offset.x = -cell->size().x;
        }
        bool in_box = isCursorIndexInBox(_selectIndex-1, cell);
        auto sender = in_box ? _cursor : _container;
        auto moveby = Action::New<MoveBy>(sender, offset * (in_box ? 1 : -1), 0.12f);
        auto call = Action::New<CallBackVoid>([this]{
            this->_scrolling = false;
            this->_selectIndex--;
        });
        auto action = Action::Ptr(new Sequence({moveby, call}));
        action->setName("TableWidget::moveCursor");
        this->runAction(action);
        this->_scrolling = true;
    }

mge_end