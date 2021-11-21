//
// Created by baifeng on 2021/11/19.
//

#include "gridmap.h"
#include <assert.h>

mge_begin

    //=====================================================================================

    GridMapLayer::GridMapLayer():_minIndex(0), _maxIndex(0) {

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
    }

    void GridMapLayer::checkTiles(GridMapWidget* gridmap) {
        auto& dirs = gridmap->getCamera()->move_dirs();
        for (auto const& dir : dirs) {
            if (dir == GridMapCamera::MOVE_UP) {
                // 摄影机上移，移除底部，补全顶部
            } else if (dir == GridMapCamera::MOVE_DOWN) {
                // 摄影机下移，移除顶部，补全底部
            } else if (dir == GridMapCamera::MOVE_RIGHT) {
                // 摄影机右移，移除左边，补全右边
            } else if (dir == GridMapCamera::MOVE_LEFT) {
                // 摄影机左移，移除右边，补全左边
            }
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

        auto layer_number = _dataSource->numberOfLayersInWidget(this);
        for (int i = 0; i < layer_number; ++i) {
            auto layer = New<GridMapLayer>();
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
                    auto cell = _dataSource->tileWidgetAtPosition(this, i, {x, y});
                    cell->setPosition(x * tile_size.x, y * tile_size.y);
                    cell->setSize(tile_size.to<float>());

                    auto tile_view = cell->to<TileWidget>();
                    auto tile_index = y * map_size.x + x;
                    tile_view->setTileIndex(tile_index);

                    auto& tileLayer = _tileLayers[i];
                    if (cell->parent() == nullptr) {
                        tileLayer->addChild(cell);
                        tileLayer->enqueueTile(cell);
                    }
                    tileLayer->tagTile(tile_index, tile_view);
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
            layer->checkTiles(this);
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