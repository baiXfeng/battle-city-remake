//
// Created by baifeng on 2021/11/17.
//

#include "widget_ex.h"
#include "action.h"
#include <assert.h>

mge_begin

    //=====================================================================================

    TableWidget::TableWidget():
    _direction(Vertical),
    _dataSource(nullptr),
    _container(nullptr),
    _cursor(nullptr),
    _selectIndex(0),
    _scrolling(false),
    _genAllCells(false),
    _moveAnimate(false) {
        addChild(Ptr(_container = new WindowWidget));
        addChild(Ptr(_cursor = new MaskWidget({0, 0, 0, 200})));
    }

    void TableWidget::setDirection(Direction dir) {
        _direction = dir;
    }

    TableWidget::Direction TableWidget::getDirection() const {
        return _direction;
    }

    void TableWidget::setDataSource(DataSource* data_source) {
        _dataSource = data_source;
    }

    TableWidget::DataSource* TableWidget::getDataSource() const {
        return _dataSource;
    }

    size_t TableWidget::getCursorIndex() const {
        return _selectIndex;
    }

    void TableWidget::startMoveCursor(MoveDirection dir, bool animate) {
        Action::Ptr call;
        _moveAnimate = animate;
        if (dir == MOVE_NEXT) {
            this->moveCursorNext(animate);
            call = Action::New<CallBackVoid>([this]{
                this->moveCursorNext(_moveAnimate, 0.045f);
            });
        } else if (dir == MOVE_PREV) {
            this->moveCursorPrev(animate);
            call = Action::New<CallBackVoid>([this]{
                this->moveCursorPrev(_moveAnimate, 0.045f);
            });
        } else {
            assert(false && "TableWidget::startMoveCursor error.");
        }
        auto delay = Action::New<Delay>(0.5f);
        auto delay1 = Action::New<Delay>(0.05f);
        auto sequence = Action::Ptr(new Sequence({call, delay1}));
        auto repeat = Action::New<Repeat>(sequence);
        auto action = Action::Ptr(new Sequence({delay, repeat}));
        action->setName("TableWidget::MoveCursorAnimation");
        stopAction("TableWidget::MoveCursorAnimation");
        runAction(action);
    }

    void TableWidget::stopMoveCursor() {
        stopAction("TableWidget::MoveCursorAnimation");
    }

    Widget::Ptr TableWidget::dequeueCell() {
        if (_idleCells.empty()) {
            return nullptr;
        }
        auto r = _idleCells.back();
        _idleCells.pop_back();
        return r;
    }

    void TableWidget::onUpdate(float delta) {
        if (_busyCells.empty() or !_scrolling or _genAllCells) {
            return;
        }
        checkCells();
    }

    void TableWidget::checkCells() {
        if (Vertical == _direction) {
            int offset = int(_container->position().y - _prevPosition.y);
            if (offset >= 1) {
                insertCellAt(HEAD);
                removeCellAt(TAIL);
            } else if (offset <= -1) {
                insertCellAt(TAIL);
                removeCellAt(HEAD);
            }
        } else {
            int offset = int(_container->position().x - _prevPosition.x);
            if (offset >= 1) {
                insertCellAt(HEAD);
                removeCellAt(TAIL);
            } else if (offset <= -1) {
                insertCellAt(TAIL);
                removeCellAt(HEAD);
            }
        }
    }

    void TableWidget::reload_data() {
        reload_data(false);
    }

    void TableWidget::reload_data(bool gen_all_cells) {
        assert(_dataSource != nullptr && "TableWidget::reload_data fail.");

        _rectList.clear();
        _idleCells.clear();
        _busyCells.clear();
        _container->removeAllChildren();
        _genAllCells = gen_all_cells;

        Vector2f containerSize;
        for (int i = 0; i < _dataSource->numberOfCellsInWidget(this); ++i) {
            auto cellSize = _dataSource->cellSizeForIndex(this, i);
            _rectList.push_back({
                int(containerSize.x),
                int(containerSize.y),
                int(cellSize.x),
                int(cellSize.y),
            });
            if (_direction == Vertical) {
                containerSize.y += cellSize.y;
            } else {
                containerSize.x += cellSize.x;
            }
        }
        _selectIndex = 0;
        _container->setSize(containerSize);
        _container->setPosition(0.0f, 0.0f);

        int nextIndex = 0;
        Vector2f position;
        for (nextIndex = 0; nextIndex < _dataSource->numberOfCellsInWidget(this); ++nextIndex) {
            auto cellSize = _dataSource->cellSizeForIndex(this, nextIndex);
            auto cell = Ptr(_dataSource->cellWidgetAtIndex(this, nextIndex));
            cell->setVisible(true);
            cell->setSize(cellSize.to<float>());
            cell->setPosition(position);
            cell->to<CellWidget>()->setCellIndex(nextIndex);
            _busyCells.push_back(cell);
            _container->addChild(cell);
            if (_selectIndex == nextIndex) {
                _cursor->setPosition(position);
                _cursor->setSize(cell->size());
            }
            if (_direction == Vertical) {
                position.y += cellSize.y;
                if (!_genAllCells) {
                    if (_container->position().y + position.y >= this->size().y) {
                        break;
                    }
                }
            } else {
                position.x += cellSize.x;
                if (!_genAllCells) {
                    if (_container->position().x + position.x >= this->size().x) {
                        break;
                    }
                }
            }
        }
        if (_idleCells.empty()) {
            auto cell = Ptr(_dataSource->cellWidgetAtIndex(this, ++nextIndex));
            cell->setVisible(false);
            _idleCells.push_back(cell);
            _container->addChild(cell);
        }

        signal(DID_SCROLL)(this);
    }

    void TableWidget::moveCursorNext(bool animate, float duration) {
        if (_selectIndex + 1 >= _dataSource->numberOfCellsInWidget(this) or this->_scrolling) {
            return;
        }
        _prevPosition = _container->position();

        auto const& rect = _rectList[ _selectIndex + 1 ];
        Vector2f offset;
        bool in_box = false;
        if (Vertical == _direction) {
            int y = _container->position().y + rect.y;
            if (y >= 0 and y + rect.h <= size().y) {
                in_box = true;
            }
            offset.y = rect.h;
        } else {
            int x = _container->position().x + rect.x;
            if (x >= 0 and x + rect.w <= size().x) {
                in_box = true;
            }
            offset.x = rect.w;
        }
        auto sender = in_box ? _cursor : _container;
        if (animate) {
            auto moveby = Action::New<MoveTo>(sender, sender->position() + offset * (in_box ? 1 : -1), duration);
            auto call = Action::New<CallBackVoid>([this]{
                this->_scrolling = false;
                this->_selectIndex++;
                this->signal(DID_SCROLL)(this);
            });
            auto action = Action::Ptr(new Sequence({moveby, call}));
            action->setName("TableWidget::moveCursor");
            this->runAction(action);
            this->_scrolling = true;
        } else {
            sender->setPosition(sender->position() + offset * (in_box ? 1 : -1));
            this->_selectIndex++;
            this->checkCells();
            this->signal(DID_SCROLL)(this);
        }
    }

    void TableWidget::moveCursorPrev(bool animate, float duration) {
        if (_selectIndex - 1 <= -1 or this->_scrolling) {
            return;
        }
        _prevPosition = _container->position();

        auto const& rect = _rectList[ _selectIndex - 1 ];
        Vector2f offset;
        bool in_box = false;
        if (Vertical == _direction) {
            int y = _container->position().y + rect.y;
            if (y >= 0 and y + rect.h <= size().y) {
                in_box = true;
            }
            offset.y = -rect.h;
        } else {
            int x = _container->position().x + rect.x;
            if (x >= 0 and x + rect.w <= size().x) {
                in_box = true;
            }
            offset.x = -rect.w;
        }
        auto sender = in_box ? _cursor : _container;
        if (animate) {
            auto moveby = Action::New<MoveTo>(sender, sender->position() + offset * (in_box ? 1 : -1), duration);
            auto call = Action::New<CallBackVoid>([this] {
                this->_scrolling = false;
                this->_selectIndex--;
                this->signal(DID_SCROLL)(this);
            });
            auto action = Action::Ptr(new Sequence({moveby, call}));
            action->setName("TableWidget::moveCursor");
            this->runAction(action);
            this->_scrolling = true;
        } else {
            sender->setPosition(sender->position() + offset * (in_box ? 1 : -1));
            this->_selectIndex--;
            this->checkCells();
            this->signal(DID_SCROLL)(this);
        }
    }

    void TableWidget::insertCellAt(CellPosition p) {
        if (p == HEAD) {
            auto& cell = _busyCells.front();
            int firstIndex = cell->to<CellWidget>()->getCellIndex();
            if (firstIndex == 0) {
                return;
            }
            bool insert = false;
            if (Vertical == _direction) {
                insert = _container->position().y + cell->position().y >= 1;
            } else {
                insert = _container->position().x + cell->position().x >= 1;
            }
            if (insert) {
                auto nextIndex = firstIndex - 1;
                auto new_cell = _dataSource->cellWidgetAtIndex(this, nextIndex);
                auto& rect = _rectList[nextIndex];
                new_cell->setVisible(true);
                new_cell->setSize({float(rect.w), float(rect.h)});
                new_cell->setPosition({float(rect.x), float(rect.y)});
                new_cell->to<CellWidget>()->setCellIndex(nextIndex);
                _busyCells.push_front(new_cell);
                if (new_cell->parent() == nullptr) {
                    _container->addChild(new_cell);
                }
            }
        } else {
            auto& cell = _busyCells.back();
            int firstIndex = cell->to<CellWidget>()->getCellIndex();
            if (firstIndex + 1 >= _dataSource->numberOfCellsInWidget(this)) {
                return;
            }
            bool insert = false;
            if (Vertical == _direction) {
                insert = size().y - (_container->position().y + cell->position().y + cell->size().y) >= 1;
            } else {
                insert = size().x - (_container->position().x + cell->position().x + cell->size().x) >= 1;
            }
            if (insert) {
                auto nextIndex = firstIndex + 1;
                auto new_cell = _dataSource->cellWidgetAtIndex(this, nextIndex);
                auto& rect = _rectList[nextIndex];
                new_cell->setVisible(true);
                new_cell->setSize({float(rect.w), float(rect.h)});
                new_cell->setPosition({float(rect.x), float(rect.y)});
                new_cell->to<CellWidget>()->setCellIndex(nextIndex);
                _busyCells.push_back(new_cell);
                if (new_cell->parent() == nullptr) {
                    _container->addChild(new_cell);
                }
            }
        }
    }

    void TableWidget::removeCellAt(CellPosition p) {
        if (p == HEAD) {
            auto& cell = _busyCells.front();
            bool remove = false;
            if (Vertical == _direction) {
                remove = _container->position().y + cell->position().y + cell->size().y <= -1;
            } else {
                remove = _container->position().x + cell->position().x + cell->size().x <= -1;
            }
            if (remove) {
                cell->setVisible(false);
                _busyCells.pop_front();
                _idleCells.push_back(cell);
            }
        } else {
            auto& cell = _busyCells.back();
            bool remove = false;
            if (Vertical == _direction) {
                remove = int(_container->position().y + cell->position().y) >= int(size().y + 1);
            } else {
                remove = int(_container->position().x + cell->position().x) >= int(size().x + 1);
            }
            if (remove) {
                cell->setVisible(false);
                _busyCells.pop_back();
                _idleCells.push_back(cell);
            }
        }
    }

mge_end