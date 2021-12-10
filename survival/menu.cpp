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

namespace dungeon {
    float roundm(float n, float m) {
        return floor((n + m - 1) / m) * m;
    }
    float random_normalized() {
        return Random::get(0.0f, 1.0f);
    }
    Vector2f getRandomPointInCircle(float radius, Vector2i const& size = {4, 4}) {
        auto t = 2 * PI * random_normalized();
        auto r = sqrt(random_normalized());
        return Vector2f{
                roundm(radius * r * cos(t), size.x),
                roundm(radius * r * sin(t), size.y)
        };
    }
}

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

void RandomRoomView::Room::modify() {
    notify(&Widget::setPosition, Vector2i{r.x, r.y}.to<float>());
    notify(&Widget::setSize, Vector2i{r.w, r.h}.to<float>());
}

void RandomRoomView::Room::color(SDL_Color const& c) {
    notify(&MaskWidget::setColor, c);
}

RandomRoomView::RandomRoomView():_roomVertex(nullptr) {
    addChild(Ptr(new MaskBoxWidget({255, 255, 255, 255})));
    _world = b2WorldSugar::CreateWorld();
    _window = _children.front().get();
    _window->setAnchor(0.5f, 0.5f);
    _window->setPosition(size().x*0.5f, size().y*0.5f);
    this->rebuild();
    _step = 0;
}

RandomRoomView::~RandomRoomView() {
    delete _world;
    if (_roomVertex) {
        delete [] _roomVertex;
        _roomVertex = nullptr;
    }
}

void RandomRoomView::rebuild() {

    _grid.clear();
    _rooms.clear();
    _mainRoom.clear();
    _passRoom.clear();
    _corridors.clear();

    _edges.clear();
    _roomIdx.clear();

    _links.clear();
    _corridorsRects.clear();

    _window->removeAllChildren();

    auto body = _world->GetBodyList();
    while (body) {
        auto next = body->GetNext();
        _world->DestroyBody(body);
        body = next;
    }

    // 限制地图尺寸
    Vector2i grid_size{200, 200};
    int grid_count = grid_size.x * 2;
    _grid.resize(grid_size, 0);
    _rooms.reserve(grid_count);
    _window->setSize((_grid.size() * getTileSize()).to<float>());

    // 生成房间
    int const big_room_num = 20;
    int const middle_room_num = 50;
    int const small_room_num = 130;

    GenRoom(big_room_num, {20, 20}, {38, 38}, true);
    queryMainRoom();
    if (_mainRoom.size() < (big_room_num >> 1)) {
        rebuild();
        return;
    }
    GenRoom(middle_room_num, {12, 12}, {22, 22});
    GenRoom(small_room_num, {6, 6}, {16, 16});
}

void RandomRoomView::alignRooms() {
    Vector2i min, max;
    auto tile_size = getTileSize();
    for (int i = 0; i < _rooms.size(); ++i) {
        auto& r = _rooms[i].r;
        r.x = int(r.x / tile_size) * tile_size;
        r.y = int(r.y / tile_size) * tile_size;
        if (r.x <= min.x) {
            min.x = r.x;
        } else if (r.x + r.w >= max.x) {
            max.x = r.x + r.w;
        }
        if (r.y <= min.y) {
            min.y = r.y;
        } else if (r.y + r.h >= max.y) {
            max.y = r.y + r.h;
        }
    }
    for (int i = 0; i < _rooms.size(); ++i) {
        auto &r = _rooms[i].r;
        r.x += abs(min.x);
        r.y += abs(min.y);
        _rooms[i].modify();
    }
    _window->setSize(max.x-min.x, max.y-min.y);
    _window->performLayout();
}

void RandomRoomView::queryMainRoom() {
    int const room_size = 16;
    float const scale = 1.25f;
    auto tile_size = getTileSize();
    int const target_size = room_size * scale * tile_size;

    _mainRoom.clear();
    for (int i = 0; i < _rooms.size(); ++i) {
        auto& r = _rooms[i].r;
        if (r.w >= target_size + 1 or r.h >= target_size + 1) {
            auto room = &_rooms[i];
            room->color({255, 0, 0, 255});
            _mainRoom.push_back(room);
        }
    }
}

void RandomRoomView::makeGraph() {
    if (_mainRoom.empty()) {
        return;
    }
    if (_roomVertex) {
        delete [] _roomVertex;
        _roomVertex = nullptr;
    }
    _roomVertex = new RoomVertex[_mainRoom.size()];
    for (int i = 0; i < _mainRoom.size(); ++i) {
        auto& r = _mainRoom[i]->r;
        _roomVertex[i].room = _mainRoom[i];
        _roomVertex[i].x = r.x + (r.w >> 1);
        _roomVertex[i].y = r.y + (r.h >> 1);
    }
    auto idb = IDelaBella::Create();
    idb->Triangulate(_mainRoom.size(), &_roomVertex->x, &_roomVertex->y, sizeof(RoomVertex));
    _edgeGraph = dungeon::create_edge_graph_with_delaunay_triangulate(idb, [this](int start, int end){
        auto& first = _roomVertex[start];
        auto& second = _roomVertex[end];
        mge::Vector2f pt1{first.x, first.y};
        mge::Vector2f pt2{second.x, second.y};
        return pt1.distance(pt2) * 10;
    });
    for (int i = 0; i < _edgeGraph.size(); ++i) {
        auto& edge = _edgeGraph[i];
        auto first = &_roomVertex[edge.start];
        auto second = &_roomVertex[edge.end];
        _edges.push_back({
            {first->x, first->y},
            {second->x, second->y},
        });
    }
    idb->Destroy();
}

void RandomRoomView::makeMiniSpanTree() {
    _edgePathGraph = dungeon::mini_span_tree_kruskal(_edgeGraph, _mainRoom.size());
    _edges.clear();
    for (int i = 0; i < _edgePathGraph.size(); ++i) {
        auto& edge = *_edgePathGraph[i];
        auto v0 = &_roomVertex[edge.start];
        auto v1 = &_roomVertex[edge.end];
        _edges.push_back({
            {v0->x, v0->y},
            {v1->x, v1->y},
        });
    }
}

void RandomRoomView::addSomeEdge() {
    std::map<int, bool> tag;
    for (int i = 0; i < _edgePathGraph.size(); ++i) {
        tag[_edgePathGraph[i]->id] = true;
    }
    int const max_count = _edgeGraph.size() * 0.15f;
    int count = 0;
    for (int i = 0; i < _edgeGraph.size(); ++i) {
        auto& e = _edgeGraph[i];
        if (tag[e.id]) {
            continue;
        }
        if (Random::get(0, 100) < 50) {
            continue;
        }
        auto v0 = &_roomVertex[e.start];
        auto v1 = &_roomVertex[e.end];
        _edges.push_back({
            {v0->x, v0->y},
            {v1->x, v1->y},
        });
        tag[e.id] = true;
        if (++count >= max_count) {
            return;
        }
    }
}

#include "common/quadtree.h"

void RandomRoomView::makeCorridor() {

    _edges.clear();

    //QuadTree<Room*> quadtree(0);

    auto tile_size = getTileSize();
    for (auto& edge : _edgePathGraph) {
        auto& first = _roomVertex[edge->start];
        auto& second = _roomVertex[edge->end];

        auto left_x = std::max(first.room->r.x, second.room->r.x);
        auto right_x = std::min(first.room->r.x + first.room->r.w, second.room->r.x + second.room->r.w);
        if ((right_x - left_x) / tile_size >= 3) {
            auto r0 = first.room->r.y < second.room->r.y ? first.room : second.room;
            auto r1 = first.room->r.y > second.room->r.y ? first.room : second.room;
            float x = (right_x - left_x) * 0.5f + left_x;
            float y0 = r0->r.y + r0->r.h;
            float y1 = r1->r.y;
            if (int(y1-y0) == 0) {
                _links.push_back(NONE_LINE);
                continue;
            }
            _links.push_back(VERTICAL_LINE);
            _edges.push_back({
                {x, y0},
                {x, y1},
            });
            continue;
        }

        auto up_y = std::max(first.room->r.y, second.room->r.y);
        auto down_y = std::min(first.room->r.y + first.room->r.h, second.room->r.y + second.room->r.h);
        if ((down_y - up_y) / tile_size >= 3) {
            auto r0 = first.room->r.x < second.room->r.x ? first.room : second.room;
            auto r1 = first.room->r.x > second.room->r.x ? first.room : second.room;
            float y = (down_y - up_y) * 0.5f + up_y;
            float x0 = r0->r.x + r0->r.w;
            float x1 = r1->r.x;
            if (int(x1-x0) == 0) {
                _links.push_back(NONE_LINE);
                continue;
            }
            _links.push_back(HORIZONTAL_LINE);
            _edges.push_back({
                {x0, y},
                {x1, y},
            });
            continue;
        }

        _links.push_back(TURNING_LINE);
        Vector2i left_top{
            (int)std::min(first.x, second.x),
            (int)std::min(first.y, second.y),
        };
        Vector2i right_bottom{
            (int)std::max(first.x, second.x),
            (int)std::max(first.y, second.y),
        };
        _corridorsRects.push_back({
            left_top.x, left_top.y,
            right_bottom.x - left_top.x, right_bottom.y - left_top.y,
        });
    }
}

void RandomRoomView::GenRoom(int room_size, mge::Vector2i const& min_size, mge::Vector2i const& max_size, bool check_overlap) {
    auto tile_size = getTileSize();
    int room_count = 0;
    while (room_count < room_size) {
        auto pos = dungeon::getRandomPointInCircle(_grid.size().x * 0.5f - 5);
        int width = Random::get(min_size.x, max_size.x);
        int height = Random::get(min_size.y, max_size.y);
        int x = (_grid.size().x >> 1) + pos.x;
        int y = (_grid.size().y >> 1) + pos.y;
        RectI ret{
                x - (width >> 1),
                y - (height >> 1),
                width, height,
        };
        if (ret.x <= -1) {
            ret.x = 0;
        } else if (ret.x + ret.w >= _grid.size().x) {
            ret.x = _grid.size().x - ret.w;
        }
        if (ret.y <= -1) {
            ret.y = 0;
        } else if (ret.y + ret.h >= _grid.size().y) {
            ret.y = _grid.size().y - ret.h;
        }
        RectI room = {
                ret.x * tile_size,
                ret.y * tile_size,
                ret.w * tile_size,
                ret.h * tile_size,
        };
        if (check_overlap) {
            if (isRoomOverlap(room)) {
                continue;
            }
        }
        addRoom(room);
        room_count++;
    }
}

void RandomRoomView::addRoom(mge::RectI const& r) {
    SDL_Color color[7] = {
            {255, 128, 128, 255},
            {0, 255, 0, 255},
            {0, 0, 255, 255},
            {255, 255, 0, 255},
            {0, 255, 255, 255},
            {255, 0, 255, 255},
            {128, 255, 128, 255},
    };

    _rooms.push_back({(int)_rooms.size()+1, r});

    auto& room = _rooms.back();
    auto mask = Ptr(new MaskWidget(color[_rooms.size() % 7]));
    _window->addChild(mask);
    room.add(mask->to<MaskWidget>());
    room.modify();

    auto body = b2BodySugar::CreateBody(_world, b2_dynamicBody);
    b2BodySugar sugar(body);
    sugar.addBoxShape({r.w, r.h});
    sugar.setTransform({
        r.x + r.w * 0.5f,
        r.y + r.h * 0.5f,
    });
    body->SetFixedRotation(true);
    _roomIdx[body] = _rooms.size() - 1;
}

bool RandomRoomView::isRoomOverlap(mge::RectI const& r) const {
    for (auto& room : _rooms) {
        if (isCollision(room.r, r)) {
            return true;
        }
    }
    return false;
}

int RandomRoomView::getTileSize() const {
    return size().y / _grid.size().y * 0.8f;
}

void RandomRoomView::onUpdate(float delta) {
    if (_step != 0) {
        return;
    }
    _world->Step(delta, 100.0f, 100.0f);
    b2BodySugar sugar;
    bool sleep = true;
    for (auto body = _world->GetBodyList(); body; body = body->GetNext()) {
        if (body->IsAwake()) {
            sleep = false;
        }
        sugar.reset(body);
        auto index = _roomIdx[body];
        auto position = sugar.getPixelPosition();
        auto& room = _rooms[index];
        room.r.x = position.x - (room.r.w >> 1);
        room.r.y = position.y - (room.r.h >> 1);
        room.modify();
    }
    _window->performLayout();
}

void RandomRoomView::draw(SDL_Renderer* renderer) {
    DrawColor dc(renderer);
    /*
    for (int i = 0; i < _rooms.size(); ++i) {
        dc.setColor(color[i % 7]);
        auto& r = _rooms[i].r;
        SDL_Rect dstrect{r.x, r.y, r.w, r.h};
        SDL_RenderFillRect(renderer, &dstrect);
    }
    for (int i = 0; i < _mainRoom.size(); ++i) {
        dc.setColor({255, 0, 0, 255});
        auto& r = _mainRoom[i]->r;
        SDL_Rect dstrect{r.x, r.y, r.w, r.h};
        SDL_RenderFillRect(renderer, &dstrect);
    }
     */
    this->onChildrenDraw(renderer);
    auto& position = _window->global_position();
    for (int i = 0; i < _edges.size(); ++i) {
        auto& e = _edges[i];
        auto& pt0 = e[0];
        auto& pt1 = e[1];
        dc.setColor({255, 255, 0, 255});
        SDL_RenderDrawLineF(renderer, position.x + pt0.x, position.y + pt0.y, position.x + pt1.x, position.y + pt1.y);
    }
    for (auto& r: _corridorsRects) {
        dc.setColor({0, 255, 0, 255});
        SDL_Rect dstrect{int(position.x) + r.x, int(position.y) + r.y, r.w, r.h};
        SDL_RenderDrawRect(renderer, &dstrect);
    }
}

void RandomRoomView::onButtonDown(int key) {
    if (key == KeyCode::X) {
        this->rebuild();
        _step = 0;
    } else if (key == KeyCode::A) {
        switch (_step) {
            case 0:
                this->alignRooms();
                break;
            case 1:
                this->makeGraph();
                break;
            case 2:
                this->makeMiniSpanTree();
                break;
            case 3:
                this->addSomeEdge();
                break;
            case 4:
                this->makeCorridor();
                break;
            default:
                return;
        }
        _step++;
    }
}
