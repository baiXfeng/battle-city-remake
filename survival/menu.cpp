//
// Created by baifeng on 2021/11/16.
//

#include "menu.h"
#include "common/loadres.h"
#include "common/game.h"
#include "common/widget_ex.h"
#include "common/gridmap.h"
#include "common/collision.h"
#include "common/render.h"
#include "common/physics.h"
#include "dungeon/generate.h"
#include "effolkronium/random.hpp"
#include <math.h>

using namespace mge;

//=====================================================================================

int _colorIndex = 9;

class WeaponIconCell : public CellWidget {
public:
    WeaponIconCell() {
        SDL_Color color[7] = {
                {255, 0, 0, 255},
                {0, 255, 0, 255},
                {0, 0, 255, 255},
                {255, 255, 0, 255},
                {0, 255, 255, 255},
                {255, 0, 255, 255},
                {0, 0, 0, 255},
        };
        auto mask = Ptr(new mge::MaskWidget(color[_colorIndex++ % 7]));
        addChild(mask);
        _mask = mask->to<MaskWidget>();
    }
private:
    void onModifySize(Vector2f const& size) override {
        //_mask->setPositionX(10);
        _mask->setSize(size-Vector2f{20, 0});
    }
private:
    MaskWidget* _mask;
};

class WeaponListView : public TableWidget, public WidgetDataSource {
public:
    WeaponListView() {
        auto icon_texture = res::load_texture(_game.renderer(), "assets/survival/icon-box.png");
        auto image = New<ImageWidget>(icon_texture);
        addChild(image);

        setAnchor(0.5f, 0.5f);
        setPosition(size().x * 0.5f, size().y * 0.35f);
        setSize(80, 80);
        setDirection(Horizontal);
        setDataSource(this);
        //reload_data(true);

        _cursor->setVisible(false);
    }
private:
    size_t numberOfCellsInWidget(TableWidget* sender) override {
        return 10;
    }
    Vector2i cellSizeForIndex(TableWidget* sender, size_t index) override {
        return {100, 80};
    }
    Widget::Ptr cellWidgetAtIndex(TableWidget* sender, size_t index) override {
        auto cell = sender->dequeueCell();
        if (cell == nullptr) {
            cell.reset(new WeaponIconCell);
        }
        return cell;
    }
};

WeaponSelectView::WeaponSelectView() {
    auto mask = Ptr(new mge::MaskWidget({255, 255, 255, 255}));
    addChild(mask);

    auto table = New<WeaponListView>();
    addChild(table);
    _weaponView = table->to<WeaponListView>();

    auto font = res::load_ttf_font("assets/fonts/prstart.ttf", 22);
    font->setColor({0, 0, 0, 255});
    auto label = TTFLabel::New("", font, {0.5f, 0.5f});
    label->setPosition(size().x * 0.5f, size().y * 0.65f);
    label->setName("label");
    addChild(label);

    _weaponView->connect(_weaponView->DID_SCROLL, [this](Widget* sender) {
        auto label = this->find("label")->to<TTFLabel>();
        label->setString(std::string("ITEM ") + std::to_string(_weaponView->getCursorIndex()+1));
    });
    _weaponView->reload_data(true);
}

void WeaponSelectView::onButtonDown(int key) {
    if (key == KeyCode::LEFT) {
        _weaponView->startMoveCursor(_weaponView->MOVE_PREV);
    } else if (key == KeyCode::RIGHT) {
        _weaponView->startMoveCursor(_weaponView->MOVE_NEXT);
    }
}

void WeaponSelectView::onButtonUp(int key) {
    if (key == KeyCode::LEFT) {
        _weaponView->stopMoveCursor();
    } else if (key == KeyCode::RIGHT) {
        _weaponView->stopMoveCursor();
    }
}

//=====================================================================================

class WorldTileCell : public TileWidget {
public:
    WorldTileCell() {

        auto mask = Ptr(new mge::MaskWidget({}));
        addChild(mask);
        _mask = mask->to<MaskWidget>();

        auto font = res::load_ttf_font("assets/fonts/prstart.ttf", 10);
        auto label = TTFLabel::New("", font, {0.5f, 0.5f});
        //label->setVisible(false);
        //addChild(label);
        //_label = label->to<TTFLabel>();
    }
    void setPos(Vector2i const& p) {
        //_label->setString(std::to_string(p.x) + "," + std::to_string(p.y), {180, 180, 180, 255});
    }
    void setTile(int id) {
        SDL_Color color[] = {
                {65, 105, 225, 255},
                {30, 144, 255, 255},
                {175, 238, 238, 255},
                {0, 250, 154, 255},
        };
        _mask->setColor(color[(id-1) % 4]);
    }
private:
    void onModifySize(Vector2f const& size) override {
        _mask->setSize(size);
        //_label->setPosition(size.x*0.5f, size.y*0.5f);
    }
private:
    MaskWidget* _mask;
    TTFLabel* _label;
};

class Cell {
public:
    enum Type {
        NONE = 0,
        ROOM,
        AISLE,
    };
public:
    Cell():_type(NONE) {}
    virtual ~Cell() {}
public:
    void setType(Type t) {
        _type = t;
    }
    Type getType() const {
        return _type;
    }
protected:
    Type _type;
};

class Room : public Cell {
public:
    Room() {
        setType(ROOM);
    }
    Room(uint32_t id, RectI const& r):id(id), rect(r) {}
public:
    uint32_t id;
    RectI rect;
};

class Aisle : public Cell {
    typedef std::vector<Vector2i> Points;
public:
    Aisle() {
        setType(AISLE);
    }
public:
    uint32_t id;
    Points points;
};

using Random = effolkronium::random_static;

class WorldTileMap : public GridMapWidget, public GridMapDataSource {
public:
    enum {
        NONE = 0,
        SEA,
        RIVER,
        LAND,
        TREE,
    };
    WorldTileMap() {
        buildMap();
        setDataSource(this);
    }
    void buildMap() {
        _map.resize(80, 80, 1);

        for (int i = 0; i < 50; ++i) {
            auto r = dungeon::getRandomPointInCircle(40);
            printf("x = %f, y = %f\n", r.x, r.y);
        }

        std::map<uint32_t, Room> room_pool;
        uint32_t room_id = 0;
        for (int i = 0; i < 100; ++i) {
            int width = rand() % 12 + 4;
            int height = rand() % 12 + 4;
            width += width % 2;
            height += height % 2;
            int x = rand() % _map.size().x + 4;
            int y = rand() % _map.size().y + 4;
            if (x + width >= _map.size().x - 4) {
                x = _map.size().x - 4 - width - rand() % 30;
            }
            if (y + height >= _map.size().y - 4) {
                y = _map.size().y - 4 - height - rand() % 30;
            }
            auto rect = RectI{x, y, width, height};
            room_id++;
            room_pool.insert(std::make_pair(room_id, Room{room_id, rect}));
        }

        for (auto& r : room_pool) {
            for (int y = 0; y < r.second.rect.h; ++y) {
                for (int x = 0; x < r.second.rect.w; ++x) {
                    _map.set(r.second.rect.x + x, r.second.rect.y + y, 2);
                }
            }
        }
    }
private:
    size_t numberOfLayersInWidget(GridMapWidget* sender) override {
        return 1;
    }
    Vector2i sizeOfGridMap(GridMapWidget* sender) override {
        return _map.size();
    }
    Vector2i sizeOfGridTile(GridMapWidget* sender) override {
        return {16, 16};
    }
    Widget::Ptr tileWidgetAtPosition(GridMapWidget* sender, int layerIndex, Vector2i const& position) override {
        auto cell = sender->dequeueTile(layerIndex);
        if (cell == nullptr) {
            cell = Ptr(new WorldTileCell);
        }
        auto tile = cell->to<WorldTileCell>();
        //tile->setPos(position);
        tile->setTile(_map.get(position));
        return cell;
    }
private:
    Grid<char> _map;
};

BattleWorldView::BattleWorldView() {
    addChild(Ptr(_worldMap = new WorldTileMap));

    if (false) {
        _worldMap->setAnchor(0.5f, 0.5f);
        _worldMap->setSize(520, 460);
        _worldMap->setPosition(480, 272);
        _worldMap->enableClip(true);

        //auto mask = Ptr(new mge::MaskWidget({0, 0, 0, 140}));
        //mask->setSize(_worldMap->size());
        //_worldMap->addChild(mask);
    }

    //_worldMap->getCamera()->setCameraPosition({100, 100});
    _worldMap->reload_data();
}

void BattleWorldView::onButtonDown(int key) {
    if (key == KeyCode::UP) {
        _worldMap->getCamera()->move({0.0f, -600.0f});
    } else if (key == KeyCode::DOWN) {
        _worldMap->getCamera()->move({0.0f, 600.0f});
    } else if (key == KeyCode::LEFT) {
        _worldMap->getCamera()->move({-600.0f, 0.0f});
    } else if (key == KeyCode::RIGHT) {
        _worldMap->getCamera()->move({600.0f, 0.0f});
    }else if (key == KeyCode::X) {
        _worldMap->buildMap();
        _worldMap->reload_data();
    }
}

void BattleWorldView::onButtonUp(int key) {
    if (key == KeyCode::UP) {
        _worldMap->getCamera()->move({});
    } else if (key == KeyCode::DOWN) {
        _worldMap->getCamera()->move({});
    } else if (key == KeyCode::LEFT) {
        _worldMap->getCamera()->move({});
    } else if (key == KeyCode::RIGHT) {
        _worldMap->getCamera()->move({});
    }
}

//=====================================================================================

PhysicsView::PhysicsView():_world(nullptr), _box(nullptr), _platform({0,0,0,0}) {
    _world = new b2World({0.0f, 9.8f});
    auto gB2Draw = new PhysicDrawner();
    gB2Draw->SetFlags(b2Draw::e_shapeBit);
    _world->SetDebugDraw(gB2Draw);
    _box = res::load_texture(_game.renderer(), "assets/survival/box.png")->data();
    this->addPlatform();
}

void PhysicsView::addPlatform() {
    auto body = b2BodySugar::CreateBody(_world, b2_staticBody);
    b2BodySugar sugar(body);
    sugar.addEdgeShape({0.0f, size().y-2}, {size().x, size().y-2});
    _platform = {0, int(size().y-2), int(size().x), 1};
}

void PhysicsView::addBox(mge::RectI const& r) {
    auto body = b2BodySugar::CreateBody(_world, b2_dynamicBody);
    //body->SetFixedRotation(true);
    //body->SetBullet(true);

    _bodies.push_back(body);
    _rects[body] = {r.x, r.y, r.w, r.h};
    auto& rect = _rects[body];

    b2BodySugar sugar(body);
    sugar.addBoxShape({rect.w, rect.h});
    sugar.setTransform(Vector2i{rect.x + (rect.w >> 1), rect.y + (rect.h >> 1)}.to<float>(), 45.0f);
    sugar.setRestitution(0.3f);
}

void PhysicsView::reset() {
    for (auto& body : _bodies) {
        _world->DestroyBody(body);
    }
    _bodies.clear();
    _rects.clear();
}

void PhysicsView::onButtonDown(int key) {
    if (key == KeyCode::X) {
        //_physics = true;
        this->addBox(RectF(size().x*0.5f, size().y*0.2f, 32, 32).to<int>());
    } else if (key == KeyCode::UP) {
        this->reset();
    }
}

void PhysicsView::onUpdate(float delta) {
    _world->Step(delta, 6.0f, 2.0f);
    b2BodySugar sugar;
    std::list<int> drop;
    for (auto iter = _bodies.begin(); iter != _bodies.end();) {
        auto body = *iter;
        auto& rect = _rects[body];
        sugar.reset(body);
        auto position = sugar.getPixelPosition();
        rect.x = position.x - (rect.w >> 1);
        rect.y = position.y - (rect.h >> 1);

        if (rect.x < -rect.w or rect.x > size().x) {
            _bodies.erase(iter++);
            _rects.erase(body);
            body->GetWorld()->DestroyBody(body);
        } else {
            iter++;
        }
    }
}

void PhysicsView::onDraw(SDL_Renderer* renderer) {
    if (!_box) {
        return;
    }
    if (false) {
        _world->DebugDraw();
    } else {
        b2BodySugar sugar;
        DrawColor dc(renderer);
        for (auto iter = _bodies.begin(); iter != _bodies.end(); iter++) {
            auto body = *iter;
            auto& rect = _rects[body];
            sugar.reset(body);
            SDL_RenderCopyEx(renderer, _box, nullptr, &rect, sugar.getPixelAngle(), nullptr, SDL_FLIP_NONE);
        }
        if (_platform.w and _platform.h) {
            dc.setColor({255, 255, 255, 255});
            SDL_RenderFillRect(renderer, &_platform);
        }
    };
}

//=====================================================================================

RandomRoomView::RandomRoomView():_builder(new dungeon::Builder), _data(new dungeon::lasr::Data) {
    addChild(Ptr(new MaskBoxWidget({255, 255, 255, 255})));
    _window = _children.front().get();
    _window->setAnchor(0.5f, 0.5f);
    _window->setPosition(size().x*0.5f, size().y*0.5f);
    _data->window = _window;
    _data->tile_size = 6;

    _builder->addStep(dungeon::lasr::build_rooms);
    _builder->addStep(dungeon::lasr::align_rooms);
    _builder->addStep(dungeon::lasr::make_graph);
    _builder->addStep(dungeon::lasr::make_mini_span_tree);
    _builder->addStep(dungeon::lasr::add_edge);
    _builder->addStep(dungeon::lasr::make_corridor);
    _builder->addStep(dungeon::lasr::check_corridor);
    _builder->addStep(dungeon::lasr::make_center);
    _builder->setData(*_data);
    _builder->step((_step = 0)++);
}

RandomRoomView::~RandomRoomView() {
    delete _builder;
    delete _data;
}

void RandomRoomView::updateRoomLayout() {
    b2BodySugar sugar;
    for (auto body = _data->world->GetBodyList(); body; body = body->GetNext()) {
        sugar.reset(body);
        auto index = _data->roomIdx[body];
        auto position = sugar.getPixelPosition();
        auto& room = _data->rooms[index];
        room.r.x = position.x - (room.r.w >> 1);
        room.r.y = position.y - (room.r.h >> 1);
        room.modify();
    }
    _window->performLayout();
}

void RandomRoomView::onUpdate(float delta) {
    if (_step != 1) {
        return;
    }
    _data->worldUpdate(delta);
    this->updateRoomLayout();
}

void RandomRoomView::draw(SDL_Renderer* renderer) {
    DrawColor dc(renderer);

    if (true) {
        int const tile_size = _data->tile_size;
        int max_y = _window->size().y / tile_size;
        int max_x = _window->size().x / tile_size;
        dc.setColor({255, 255, 255, 255});
        for (int y = 0; y < max_y; ++y) {
            auto& pos = _window->global_position();
            auto& size = _window->global_size();
            Vector2f pt1{
                    pos.x, pos.y + y * tile_size
            };
            Vector2f pt2{
                    pos.x + size.x, pos.y + y * tile_size
            };
            SDL_RenderDrawLine(renderer, (int)pt1.x, (int)pt1.y, (int)pt2.x, (int)pt2.y);
        }
        for (int x = 0; x < max_x; ++x) {
            auto& pos = _window->global_position();
            auto& size = _window->global_size();
            Vector2f pt1{
                    pos.x + x * tile_size, pos.y
            };
            Vector2f pt2{
                    pos.x + x * tile_size, pos.y + size.y
            };
            SDL_RenderDrawLine(renderer, (int)pt1.x, (int)pt1.y, (int)pt2.x, (int)pt2.y);
        }
    }

    this->onChildrenDraw(renderer);

    if (_step >= 5) {
        //return;
    }
    auto& position = _window->global_position();
    for (int i = 0; i < _data->edges.size(); ++i) {
        if (_data->invalidEdge[i]) {
            continue;
        }
        auto& e = _data->edges[i];
        auto& pt0 = e[0];
        auto& pt1 = e[1];
        dc.setColor({255, 255, 0, 255});
        SDL_RenderDrawLineF(renderer, position.x + pt0.x, position.y + pt0.y, position.x + pt1.x, position.y + pt1.y);
    }
}

void RandomRoomView::onButtonDown(int key) {
    if (key == KeyCode::X) {
        _data->buildRetryCount = 0;
        _builder->step((_step = 0)++);
    } else if (key == KeyCode::A) {
        switch (_step) {
            case 1:
                if (!_data->isWorldSleep()) {
                    return;
                }
                _builder->step(_step++);
                break;
            default:
                if (_step < _builder->stepSize()) {
                    _builder->step(_step++);
                }
                break;
        }
    } else if (key == KeyCode::Y) {
        _builder->step((_step = 0)++);
        dungeon::Context c(_builder);
        dungeon::lasr::step_world(c);
        this->updateRoomLayout();
        while (_step < _builder->stepSize()) {
            _builder->step(_step++);
        }
    }
}
