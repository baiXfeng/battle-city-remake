//
// Created by baifeng on 2021/11/19.
//

#include "gridmap.h"
#include "common/collision.h"
#include <assert.h>

mge_begin

    //=====================================================================================

    GridMapLayer::GridMapLayer(GridMapWidget* gridmap, int layerIndex):_layerIndex(layerIndex), _minIndex(0), _maxIndex(0), _gridmap(gridmap) {

    }

    Widget::Ptr GridMapLayer::dequeueTile() {
        if (_idleTiles.empty()) {
            return nullptr;
        }
        auto r = _idleTiles.back();
        _idleTiles.pop_back();
        return r;
    }

    void GridMapLayer::enqueueTile(Widget::Ptr const& widget) {
        _busyTiles.push_back(widget);
    }

    void GridMapLayer::tagTile(size_t tile_index, TileWidget* tile_view) {
        _tiles[tile_index] = tile_view;
        if (tile_index <= _minIndex or _tiles.size() == 1) {
            _minIndex = tile_index;
        } else if (tile_index >= _maxIndex) {
            _maxIndex = tile_index;
        }
    }

    void GridMapLayer::makeTile(Vector2i const& tile_pos) {
        auto map_size = _gridmap->getDataSource()->sizeOfGridMap(_gridmap);
        auto tile_size = _gridmap->getDataSource()->sizeOfGridTile(_gridmap);
        auto cell = _gridmap->getDataSource()->tileWidgetAtPosition(_gridmap, _layerIndex, tile_pos);
        int x = tile_pos.x;
        int y = tile_pos.y;

        cell->setVisible(true);
        cell->setPosition(x * tile_size.x, y * tile_size.y);
        cell->setSize(tile_size.to<float>());

        auto tile_view = cell->to<TileWidget>();
        auto tile_index = y * map_size.x + x;
        tile_view->setTileIndex(tile_index);

        if (cell->parent() == nullptr) {
            this->addChild(cell);
        }
        this->enqueueTile(cell);
        this->tagTile(tile_index, tile_view);
    }

    void GridMapLayer::checkTiles() {
        auto& dirs = _gridmap->getCamera()->move_dirs();
        for (auto const& dir : dirs) {
            if (dir == GridMapCamera::MOVE_UP) {
                // 摄影机上移，移除底部，补全顶部
                removeBottom();
                insertTop();
            } else if (dir == GridMapCamera::MOVE_DOWN) {
                // 摄影机下移，移除顶部，补全底部
                removeTop();
                insertBottom();
            } else if (dir == GridMapCamera::MOVE_RIGHT) {
                // 摄影机右移，移除左边，补全右边
                removeLeft();
                insertRight();
            } else if (dir == GridMapCamera::MOVE_LEFT) {
                // 摄影机左移，移除右边，补全左边
                removeRight();
                insertLeft();
            }
        }
        printf("idle = %d, busy = %d, child = %d\n", _idleTiles.size(), _busyTiles.size(), _children.size());
        printf("min_index = %d, max_index = %d\n", _minIndex, _maxIndex);
    }

    void GridMapLayer::removeTop() {
        auto view = _tiles[_minIndex];
        assert(view && "GridMapLayer::removeTop fail.");
        if (_gridmap->getCamera()->inCamera({
            int(view->position().x),
            int(view->position().y),
            int(view->size().x),
            int(view->size().y),
        })) {
            return;
        }
        auto map_size = _gridmap->getDataSource()->sizeOfGridMap(_gridmap);
        int begin_x = _minIndex % map_size.x;
        int length_x = _maxIndex % map_size.x - begin_x;
        int begin_index = _minIndex;
        for (int i = begin_index; i < begin_index + length_x + 1; ++i) {
            auto& view = _tiles[i];
            if (view == nullptr) {
                break;
            }
            view->setVisible(false);
            for (auto iter = _busyTiles.begin(); iter != _busyTiles.end(); iter++) {
                if (iter->get() == view) {
                    _idleTiles.push_back(*iter);
                    _busyTiles.erase(iter);
                    break;
                }
            }
            _tiles.erase(i);
        }
        _minIndex += map_size.x;
    }

    void GridMapLayer::removeBottom() {
        auto view = _tiles[_maxIndex];
        assert(view && "GridMapLayer::removeBottom fail.");
        if (_gridmap->getCamera()->inCamera({
            int(view->position().x),
            int(view->position().y),
            int(view->size().x),
            int(view->size().y),
        })) {
            return;
        }
        auto map_size = _gridmap->getDataSource()->sizeOfGridMap(_gridmap);
        int begin_x = _minIndex % map_size.x;
        int length_x = _maxIndex % map_size.x - begin_x;
        int begin_index = _maxIndex - length_x;
        for (int i = begin_index; i < begin_index + length_x + 1; ++i) {
            auto& view = _tiles[i];
            if (view == nullptr) {
                break;
            }
            view->setVisible(false);
            for (auto iter = _busyTiles.begin(); iter != _busyTiles.end(); iter++) {
                if (iter->get() == view) {
                    _idleTiles.push_back(*iter);
                    _busyTiles.erase(iter);
                    break;
                }
            }
            _tiles.erase(i);
        }
        _maxIndex -= map_size.x;
    }

    void GridMapLayer::removeLeft() {
        auto view = _tiles[_minIndex];
        assert(view && "GridMapLayer::removeLeft fail.");
        if (_gridmap->getCamera()->inCamera({
            int(view->position().x),
            int(view->position().y),
            int(view->size().x),
            int(view->size().y),
        })) {
            return;
        }
        auto map_size = _gridmap->getDataSource()->sizeOfGridMap(_gridmap);
        int begin_y = _minIndex / map_size.x;
        int length_y = _maxIndex / map_size.x - begin_y;
        int begin_index = _minIndex;
        int next_index = begin_index + 1;
        for (int i = 0; i < length_y + 1; ++i) {
            auto& view = _tiles[begin_index];
            if (view == nullptr) {
                break;
            }
            view->setVisible(false);
            for (auto iter = _busyTiles.begin(); iter != _busyTiles.end(); iter++) {
                if (iter->get() == view) {
                    _idleTiles.push_back(*iter);
                    _busyTiles.erase(iter);
                    break;
                }
            }
            _tiles.erase(begin_index);
            begin_index += map_size.x;
        }
        _minIndex = next_index;
    }

    void GridMapLayer::removeRight() {
        auto view = _tiles[_maxIndex];
        assert(view && "GridMapLayer::removeRight fail.");
        if (_gridmap->getCamera()->inCamera({
            int(view->position().x),
            int(view->position().y),
            int(view->size().x),
            int(view->size().y),
        })) {
            return;
        }
        auto map_size = _gridmap->getDataSource()->sizeOfGridMap(_gridmap);
        int begin_y = _minIndex / map_size.x;
        int length_y = _maxIndex / map_size.x - begin_y;
        int begin_index = _maxIndex;
        int next_index = begin_index - 1;
        for (int i = 0; i < length_y + 1; ++i) {
            auto& view = _tiles[begin_index];
            if (view == nullptr) {
                break;
            }
            view->setVisible(false);
            for (auto iter = _busyTiles.begin(); iter != _busyTiles.end(); iter++) {
                if (iter->get() == view) {
                    _idleTiles.push_back(*iter);
                    _busyTiles.erase(iter);
                    break;
                }
            }
            _tiles.erase(begin_index);
            begin_index -= map_size.x;
        }
        _maxIndex = next_index;
    }

    void GridMapLayer::insertTop() {
        auto map_size = _gridmap->getDataSource()->sizeOfGridMap(_gridmap);
        if (_minIndex - map_size.x <= -1) {
            return;
        }
        auto view = _tiles[_minIndex];
        assert(view && "GridMapLayer::insertTop fail.");
        if (not _gridmap->getCamera()->inCamera({
            int(view->position().x),
            int(view->position().y - view->size().y),
            int(view->size().x),
            int(view->size().y),
        })) {
            return;
        }
        int begin_x = _minIndex % map_size.x;
        int begin_y = _minIndex / map_size.x - 1;
        int length_x = _maxIndex % map_size.x - begin_x;
        for (int x = begin_x; x < begin_x + length_x + 1; ++x) {
            int y = begin_y;
            this->makeTile({x, y});
        }
    }

    void GridMapLayer::insertBottom() {
        auto map_size = _gridmap->getDataSource()->sizeOfGridMap(_gridmap);
        if (_maxIndex + map_size.x >= map_size.x * map_size.y) {
            return;
        }
        auto view = _tiles[_maxIndex];
        assert(view && "GridMapLayer::removeTop fail.");
        if (not _gridmap->getCamera()->inCamera({
            int(view->position().x),
            int(view->position().y + view->size().y),
            int(view->size().x),
            int(view->size().y),
        })) {
            return;
        }
        int begin_x = _minIndex % map_size.x;
        int begin_y = _maxIndex / map_size.x + 1;
        int length_x = _maxIndex % map_size.x - begin_x;
        for (int x = begin_x; x < begin_x + length_x + 1; ++x) {
            int y = begin_y;
            this->makeTile({x, y});
        }
    }

    void GridMapLayer::insertLeft() {
        auto map_size = _gridmap->getDataSource()->sizeOfGridMap(_gridmap);
        if (_minIndex % map_size.x <= 0) {
            return;
        }
        auto view = _tiles[_minIndex];
        assert(view && "GridMapLayer::insertLeft fail.");
        if (not _gridmap->getCamera()->inCamera({
            int(view->position().x - view->size().x),
            int(view->position().y),
            int(view->size().x),
            int(view->size().y),
        })) {
            return;
        }
        int begin_x = (_minIndex % map_size.x) - 1;
        int begin_y = _minIndex / map_size.x;
        int length_y = _maxIndex / map_size.x - begin_y;
        for (int i = 0; i < length_y + 1; ++i) {
            this->makeTile({begin_x, begin_y + i});
        }
    }

    void GridMapLayer::insertRight() {
        auto map_size = _gridmap->getDataSource()->sizeOfGridMap(_gridmap);
        if (_maxIndex % map_size.x >= map_size.x - 1) {
            return;
        }
        auto view = _tiles[_maxIndex];
        assert(view && "GridMapLayer::insertRight fail.");
        if (not _gridmap->getCamera()->inCamera({
            int(view->position().x + view->size().x),
            int(view->position().y),
            int(view->size().x),
            int(view->size().y),
        })) {
            return;
        }
        int begin_x = (_maxIndex % map_size.x) + 1;
        int begin_y = _minIndex / map_size.x;
        int length_y = _maxIndex / map_size.x - begin_y;
        for (int i = 0; i < length_y + 1; ++i) {
            this->makeTile({begin_x, begin_y + i});
        }
    }

    //=====================================================================================

    GridMapCamera::GridMapCamera(Widget* container):_container(container) {}

    void GridMapCamera::onUpdate(float delta) {
        _container->setPosition(_container->position() + _speed * delta);
        _moveDirs.clear();
        int x = int(_speed.x);
        if (x >= 1) {
            _moveDirs.push_back(MOVE_LEFT);
        } else if (x <= -1) {
            _moveDirs.push_back(MOVE_RIGHT);
        }
        int y = int(_speed.y);
        if (y >= 1) {
            _moveDirs.push_back(MOVE_UP);
        } else if (y <= -1) {
            _moveDirs.push_back(MOVE_DOWN);
        }
        if (_moveDirs.size()) {
            signal(DID_SCROLL)(this);
        }
    }

    void GridMapCamera::move(Vector2f const& speed) {
        _speed = speed * -1;
    }

    GridMapCamera::MoveDirs const& GridMapCamera::move_dirs() const {
        return _moveDirs;
    }

    bool GridMapCamera::inCamera(RectI const& r) const {
        return isCollision(RectI{
            int(_container->position().x + r.x),
            int(_container->position().y + r.y),
            int(r.w),
            int(r.h),
        }, RectI{
            int(position().x),
            int(position().y),
            int(size().x),
            int(size().y),
        });
    }

    //=====================================================================================

    GridMapWidget::GridMapWidget():_dataSource(nullptr) {
        addChild(Ptr(_container = new WindowWidget));
        addChild(Ptr(_camera = new GridMapCamera(_container)));
        _camera->connect(_camera->DID_SCROLL, std::bind(&GridMapWidget::checkTiles, this, std::placeholders::_1));
    }

    void GridMapWidget::setDataSource(DataSource* data_source) {
        _dataSource = data_source;
    }

    GridMapWidget::DataSource* GridMapWidget::getDataSource() const {
        return _dataSource;
    }

    GridMapLayer* GridMapWidget::getLayer(int index) const {
        return _tileLayers[index];
    }

    GridMapCamera* GridMapWidget::getCamera() const {
        return _camera;
    }

    void GridMapWidget::reload_data() {
        assert(_dataSource != nullptr && "GridMapWidget::reload_data fail.");

        _tileLayers.clear();
        _container->removeAllChildren();
        _camera->setSize(size());

        auto map_size = _dataSource->sizeOfGridMap(this);
        auto tile_size = _dataSource->sizeOfGridTile(this);
        auto view_size = map_size * tile_size;
        _container->setSize(view_size.to<float>());
        _container->setPosition(tile_size.x * -0.5f, tile_size.y * -5.5f);

        auto layer_number = _dataSource->numberOfLayersInWidget(this);
        for (int i = 0; i < layer_number; ++i) {
            auto layer = New<GridMapLayer>(this, i);
            _container->addChild(layer);
            _tileLayers.push_back(layer->to<GridMapLayer>());
        }

        auto begin_position = _container->position() * -1;
        int begin_x = begin_position.x / tile_size.x;
        int begin_y = begin_position.y / tile_size.y;
        for (int y = begin_y; y < map_size.y; ++y) {
            if (y <= -1) {
                continue;
            }
            for (int x = begin_x; x < map_size.x; ++x) {
                if (x <= -1) {
                    continue;
                }
                for (int i = 0; i < layer_number; ++i) {
                    _tileLayers[i]->makeTile({x, y});
                }
                if (_container->position().x + (x + 1) * tile_size.x >= size().x) {
                    break;
                }
            }
            if (_container->position().y + (y + 1) * tile_size.y >= size().y) {
                break;
            }
        }
    }

    void GridMapWidget::onModifySize(Vector2f const& size) {
        _camera->setSize(size);
    }

    void GridMapWidget::checkTiles(Widget* sender) {
        for (auto& layer : _tileLayers) {
            layer->checkTiles();
        }
    }

    Widget::Ptr GridMapWidget::dequeueTile(int layer) {
        assert(layer >= 0 && "GridMapWidget::dequeueTile error.");
        if (layer >= _tileLayers.size()) {
            return nullptr;
        }
        return _tileLayers[layer]->dequeueTile();
    }

mge_end