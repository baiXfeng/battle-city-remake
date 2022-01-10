//
// Created by baifeng on 2021/12/17.
//

#include "Irregular_maze_of_large_and_small_rooms.h"
#include "common/physics.h"
#include "common/collision.h"
#include "common/quadtree.h"
#include "effolkronium/random.hpp"

namespace dungeon {

    namespace lasr {

        using namespace mge;

        void Room::modify() {
            notify(&Widget::setPosition, Vector2i{r.x, r.y}.to<float>());
            notify(&Widget::setSize, Vector2i{r.w, r.h}.to<float>());
        }

        void Room::color(SDL_Color const& c) {
            notify(&MaskWidget::setColor, c);
        }

        void Room::hide() {
            each([](View* v){
                v->setVisible(false);
            });
        }

        void Room::show() {
            each([](View* v){
                v->setVisible(true);
            });
        }

        class RoomPathFinding {
        public:
            class Node {
            public:
                typedef std::shared_ptr<Node> p;
                typedef p Nodep;
                static Nodep New() {
                    auto p = new Node;
                    p->value = 0;
                    return Nodep(p);
                }
            public:
                int value;
                Nodep prev;
                std::vector<Nodep> next;
            public:
                void clear() {
                    prev = nullptr;
                    for (auto& n : next) {
                        n->clear();
                    }
                    next.clear();
                }
            };
        public:
            RoomPathFinding(Data& d): d(d) {

            }
            ~RoomPathFinding() {
                clear();
            }
        public:
            void init() {
                clear();
                initScore();
                initNode();
            }
            float get(int first, int second) {
                float ret = 0.0f;
                auto path = findPath(first, second);
                for (int i = 0; i < path.size() - 1; ++i) {
                    ret += getScore(path[i], path[i+1]);
                }
                return ret;
            }
            std::vector<int> findPath(int first, int second) {
                class TempNode {
                public:
                    typedef std::shared_ptr<TempNode> p;
                    TempNode(TempNode* parent, Node* node, int score):parent(parent), node(node), score(score) {}
                    static p New(TempNode* parent, Node* node, int score = 0) {
                        return p(new TempNode(parent, node, score));
                    }
                public:
                    TempNode* parent;
                    Node* node;
                    int score;
                };
                std::vector<TempNode::p> open_list;
                std::vector<TempNode::p> closed_list;
                TempNode* current = nullptr;

                open_list.reserve(_nodeCache.size());
                closed_list.reserve(_nodeCache.size());
                auto has_value = [](std::vector<TempNode::p> const& list, int value) {
                    for (auto& n : list) {
                        if (n->node->value == value) {
                            return true;
                        }
                    }
                    return false;
                };

                // A* find path
                open_list.push_back(TempNode::New(nullptr, _nodeCache[first].get()));
                while (!open_list.empty()) {
                    auto current_it = open_list.begin();
                    current = current_it->get();
                    for (auto iter = open_list.begin(); iter != open_list.end(); iter++) {
                        // 获取离终点最近的节点
                        if (iter->get()->score <= current->score) {
                            current_it = iter;
                            current = current_it->get();
                        }
                    }
                    if (current->node->value == second) {
                        break;
                    }
                    closed_list.push_back(*current_it);
                    open_list.erase(current_it);
                    // 加入邻近节点供下次查找
                    if (current->node->prev.get() and !has_value(closed_list, current->node->prev->value)) {
                        open_list.push_back(TempNode::New(
                                current,
                                current->node->prev.get(),
                                getNodeDistance(current->node->prev->value, second))
                        );
                    }
                    for (auto& n : current->node->next) {
                        if (!has_value(closed_list, n->value)) {
                            open_list.push_back(TempNode::New(current, n.get(), getNodeDistance(n->value, second)));
                        }
                    }
                }

                std::vector<int> ret;
                // 获取路径
                while (current != nullptr) {
                    ret.push_back(current->node->value);
                    current = current->parent;
                }
                // 路径倒序
                for (int i = 0; i < (ret.size() >> 1); ++i) {
                    int temp = ret[i];
                    ret[i] = ret[ret.size() - i - 1];
                    ret[ret.size() - i - 1] = temp;
                }
                return ret;
            }
        private:
            void clear() {
                if (_root != nullptr) {
                    _root->clear();
                    _root = nullptr;
                }
                _nodeCache.clear();
                _scoreCache.clear();
            }
            int getNodeDistance(int start, int end) {
                auto& first = d.roomVertex[start];
                auto& second = d.roomVertex[end];
                return Vector2f{first.x, first.y}.distance({second.x, second.y}) * 1000;
            }
            int getScore(int curr, int next) {
                return _scoreCache[std::min(curr, next) * 10000 + std::max(curr, next)];
            }
            void initScore() {
                for (auto& edge : d.edgeMiniPathGraph) {
                    int curr = edge->start;
                    int next = edge->end;
                    _scoreCache[std::min(curr, next) * 10000 + std::max(curr, next)] = edge->weight;
                }
            }
            void initNode() {
                _root = Node::New();
                _nodeCache[0] = _root;
                addNode(_root);
            }
            void addNode(Node::p const& prev) {
                for (auto& edge : d.edgeMiniPathGraph) {
                    if (prev->value == edge->start or prev->value == edge->end) {
                        int next = prev->value == edge->start ? edge->end : edge->start;
                        if (_nodeCache.find(next) != _nodeCache.end()) {
                            continue;
                        }
                        auto n = Node::New();
                        n->value = next;
                        n->prev = prev;
                        prev->next.push_back(n);
                        _nodeCache[next] = n;
                        addNode(n);
                    }
                }
            }
        private:
            Data& d;
            Node::p _root;
            std::map<int, Node::p> _nodeCache;
            std::map<int, float> _scoreCache;
        };

        Data::Data():
            world(mge::b2WorldSugar::CreateWorld()),
            buildRetryCount(0), roomVertex(nullptr),
            window(nullptr), tile_size(8),
            start_room(0), end_room(0), path_finding(new RoomPathFinding(*this)) {
        }

        Data::~Data() {
            delete world;
            if (roomVertex) {
                delete [] roomVertex;
                roomVertex = nullptr;
            }
        }

        void Data::worldUpdate(float delta) {
            world->Step(delta, 100.0f, 100.0f);
        }

        bool Data::isWorldSleep() const {
            for (auto body = world->GetBodyList(); body; body = body->GetNext()) {
                if (body->IsAwake()) {
                    return false;
                }
            }
            return true;
        }

        using Random = effolkronium::random_static;
        class BuildRooms {
        public:
            BuildRooms(Data& d):d(d) {}
        public:
            void build() {
                d.grid.clear();
                d.rooms.clear();
                d.mainRoom.clear();
                d.passRoom.clear();

                d.edges.clear();
                d.roomIdx.clear();
                d.linkRoom.clear();
                d.invalidEdge.clear();

                d.window->removeAllChildren();

                auto body = d.world->GetBodyList();
                while (body) {
                    auto next = body->GetNext();
                    d.world->DestroyBody(body);
                    body = next;
                }

                // 限制地图尺寸
                mge::Vector2i grid_size{100, 100};
                int grid_count = grid_size.x * 2;
                d.grid.resize(grid_size, 0);
                d.rooms.reserve(grid_count);
                d.window->setSize((d.grid.size() * d.tile_size).to<float>());

                // 生成房间
                int const big_room_num = 15;
                int const middle_room_num = 40;
                int const small_room_num = 80;

                GenRoom(d.grid.size().x * 0.5f, big_room_num, {10, 10}, {20, 20});
                queryMainRoom();
                if (d.mainRoom.size() < (big_room_num >> 1) and d.buildRetryCount <= 10) {
                    ++d.buildRetryCount;
                    build();
                    return;
                }
                GenRoom(d.grid.size().x * 0.65f, middle_room_num, {6, 6}, {16, 16});
                GenRoom(d.grid.size().x * 0.75f, small_room_num, {4, 4}, {12, 12});
            }
            void queryMainRoom() {
                int const room_size = 10;
                float const scale = 1.25f;
                auto tile_size = d.tile_size;
                int const target_size = room_size * scale * tile_size;

                d.mainRoom.clear();
                for (int i = 0; i < d.rooms.size(); ++i) {
                    auto& r = d.rooms[i].r;
                    if (r.w >= target_size + 1 or r.h >= target_size + 1) {
                        auto& room = d.rooms[i];
                        room.color({255, 0, 0, 255});
                        room.type = MAIN_ROOM;
                        d.mainRoom.push_back(&room);
                    }
                }
            }
            void GenRoom(float radius, int room_size, mge::Vector2i const& min_size, mge::Vector2i const& max_size, bool check_overlap = false) {
                auto tile_size = d.tile_size;
                int room_count = 0;
                while (room_count < room_size) {
                    auto pos = dungeon::getRandomPointInCircle(radius);
                    int width = Random::get(min_size.x, max_size.x);
                    int height = Random::get(min_size.y, max_size.y);
                    int x = (d.grid.size().x >> 1) + pos.x;
                    int y = (d.grid.size().y >> 1) + pos.y;
                    mge::RectI ret{
                            x - (width >> 1),
                            y - (height >> 1),
                            width, height,
                    };
                    if (ret.x <= -1) {
                        ret.x = 0;
                    } else if (ret.x + ret.w >= d.grid.size().x) {
                        ret.x = d.grid.size().x - ret.w;
                    }
                    if (ret.y <= -1) {
                        ret.y = 0;
                    } else if (ret.y + ret.h >= d.grid.size().y) {
                        ret.y = d.grid.size().y - ret.h;
                    }
                    mge::RectI room = {
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
                    addRoom(room, NONE_TYPE);
                    room_count++;
                }
            }
            Room& addRoom(mge::RectI const& r, RoomType type, float alpha = 1.0f, int index = -1) {
                SDL_Color color[7] = {
                        {255, 128, 128, 255},
                        {0, 255, 0, 255},
                        {0, 0, 255, 255},
                        {255, 255, 0, 255},
                        {0, 255, 255, 255},
                        {255, 0, 255, 255},
                        {128, 255, 128, 255},
                };

                d.rooms.push_back({(int)d.rooms.size()+1, type, r});

                auto& room = d.rooms.back();
                auto& c = color[2];
                if (type == TURNING_CORRIDOR or type == LINE_CORRIDOR) {
                    c = {255, 255, 255, Uint8(255 * alpha)};
                }
                auto mask = mge::Widget::Ptr(new mge::MaskWidget({c.r, c.g, c.b, Uint8(c.a * alpha)}));
                auto box = mge::Widget::Ptr(new mge::MaskBoxWidget({255, 255, 255, 255}));
                box->setSize(room.r.w, room.r.h);
                mask->addChild(box);
                d.window->addChild(mask, index == -1 ? d.window->children().size() : index);
                room.add(mask->to<mge::MaskWidget>());
                room.modify();

                auto const tile_size = d.tile_size;
                auto body = mge::b2BodySugar::CreateBody(d.world, b2_dynamicBody);
                mge::b2BodySugar sugar(body);
                sugar.addBoxShape({r.w + tile_size * 0.5f, r.h + tile_size * 0.5f});
                sugar.setTransform({
                    r.x + r.w * 0.5f,
                    r.y + r.h * 0.5f,
                    });
                body->SetFixedRotation(true);
                d.roomIdx[body] = d.rooms.size() - 1;
                return room;
            }
            bool isRoomOverlap(mge::RectI const& r) const {
                for (auto& room : d.rooms) {
                    if (mge::isCollision(room.r, r)) {
                        return true;
                    }
                }
                return false;
            }
        private:
            Data& d;
        };

        void build_rooms(Context& c) {
            BuildRooms br(mge::to<Data>(&c.data()));
            br.build();
        }

        void step_world(Context& c) {
            auto& d = mge::to<Data>(&c.data());
            auto const delta = 1 / 60.0f * 50;
            while (!d.isWorldSleep()) {
                d.worldUpdate(delta);
            }
        }

        void align_rooms(Context& c) {
            auto& d = mge::to<Data>(&c.data());
            mge::Vector2i min, max;
            auto tile_size = d.tile_size;
            for (int i = 0; i < d.rooms.size(); ++i) {
                auto& r = d.rooms[i].r;
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
            for (int i = 0; i < d.rooms.size(); ++i) {
                auto &r = d.rooms[i].r;
                r.x += abs(min.x);
                r.y += abs(min.y);
                r.x = floor(r.x / tile_size) * tile_size;
                r.y = floor(r.y / tile_size) * tile_size;
                d.rooms[i].modify();
            }
            d.window->setSize(max.x-min.x, max.y-min.y);
            d.window->performLayout();
        }

        void make_graph(Context& c) {
            auto& d = mge::to<Data>(&c.data());
            if (d.mainRoom.empty()) {
                return;
            }
            if (d.roomVertex) {
                delete [] d.roomVertex;
                d.roomVertex = nullptr;
            }
            d.roomVertex = new RoomVertex[d.mainRoom.size()];
            for (int i = 0; i < d.mainRoom.size(); ++i) {
                auto& r = d.mainRoom[i]->r;
                d.roomVertex[i].room = d.mainRoom[i];
                d.roomVertex[i].x = r.x + (r.w >> 1);
                d.roomVertex[i].y = r.y + (r.h >> 1);
            }
            auto idb = IDelaBella::Create();
            idb->Triangulate(d.mainRoom.size(), &d.roomVertex->x, &d.roomVertex->y, sizeof(RoomVertex));
            d.edgeGraph = dungeon::create_edge_graph_with_delaunay_triangulate(idb, [&d](int start, int end){
                auto& first = d.roomVertex[start];
                auto& second = d.roomVertex[end];
                mge::Vector2f pt1{first.x, first.y};
                mge::Vector2f pt2{second.x, second.y};
                return pt1.distance(pt2) * 10;
            });
            for (int i = 0; i < d.edgeGraph.size(); ++i) {
                auto& edge = d.edgeGraph[i];
                auto first = &d.roomVertex[edge.start];
                auto second = &d.roomVertex[edge.end];
                d.edges.push_back({
                    {first->x, first->y},
                    {second->x, second->y},
                    });
            }
            idb->Destroy();
        }

        void make_mini_span_tree(Context& c) {
            auto& d = mge::to<Data>(&c.data());
            d.edgePathGraph = mini_span_tree_kruskal(d.edgeGraph, d.mainRoom.size());
            d.edgeMiniPathGraph = d.edgePathGraph;
            d.edges.clear();
            for (int i = 0; i < d.edgePathGraph.size(); ++i) {
                auto& edge = *d.edgePathGraph[i];
                auto v0 = &d.roomVertex[edge.start];
                auto v1 = &d.roomVertex[edge.end];
                d.edges.push_back({
                    {v0->x, v0->y},
                    {v1->x, v1->y},
                });
            }
        }

        void add_edge(Context& c) {
            auto& d = mge::to<Data>(&c.data());
            std::map<int, bool> tag;
            for (int i = 0; i < d.edgePathGraph.size(); ++i) {
                tag[d.edgePathGraph[i]->id] = true;
            }
            int const max_count = d.edgeGraph.size() * 0.1f;
            int count = 0;
            for (int i = 0; i < d.edgeGraph.size(); ++i) {
                auto& e = d.edgeGraph[i];
                if (tag[e.id]) {
                    continue;
                }
                if (e.start == d.start_room or e.end == d.start_room) {
                    // 开始房间不添加环路
                    continue;
                }
                if (e.start == d.end_room or e.end == d.end_room) {
                    // 最终房间不添加环路
                    continue;
                }
                if (Random::get(0, 100) < 60) {
                    continue;
                }
                auto v0 = &d.roomVertex[e.start];
                auto v1 = &d.roomVertex[e.end];
                d.edgePathGraph.push_back(&e);
                d.edges.push_back({
                    {v0->x, v0->y},
                    {v1->x, v1->y},
                });
                tag[e.id] = true;
                if (++count >= max_count) {
                    return;
                }
            }
        }

        void make_corridor(Context& c) {
            auto& d = mge::to<Data>(&c.data());
            auto tile_size = d.tile_size;
            BuildRooms br(d);
            for (int i = 0; i < d.edgePathGraph.size(); ++i) {
                auto& edge = d.edgePathGraph[i];
                auto& first = d.roomVertex[edge->start];
                auto& second = d.roomVertex[edge->end];

                auto left_x = std::max(first.room->r.x, second.room->r.x);
                auto right_x = std::min(first.room->r.x + first.room->r.w, second.room->r.x + second.room->r.w);
                if ((right_x - left_x) / tile_size >= 3) {
                    auto r0 = first.room->r.y < second.room->r.y ? first.room : second.room;
                    auto r1 = first.room->r.y > second.room->r.y ? first.room : second.room;

                    float x = (right_x - left_x) * 0.5f + left_x;
                    float y0 = r0->r.y + r0->r.h - tile_size;
                    float y1 = r1->r.y + tile_size;

                    x = int(x / tile_size) * tile_size;
                    if (x - tile_size >= left_x and x + tile_size * 2 <= right_x) {
                        auto& room = br.addRoom({x-tile_size, std::min(y0, y1), tile_size*3, (int)abs(y1-y0)}, LINE_CORRIDOR, 0.6f, 0);
                        d.linkRoom.push_back({i, {&room}});
                        continue;
                    }
                }

                auto up_y = std::max(first.room->r.y, second.room->r.y);
                auto down_y = std::min(first.room->r.y + first.room->r.h, second.room->r.y + second.room->r.h);
                if ((down_y - up_y) / tile_size >= 3) {
                    auto r0 = first.room->r.x < second.room->r.x ? first.room : second.room;
                    auto r1 = first.room->r.x > second.room->r.x ? first.room : second.room;

                    float y = (down_y - up_y) * 0.5f + up_y;
                    float x0 = r0->r.x + r0->r.w - tile_size;
                    float x1 = r1->r.x + tile_size;

                    y = int(y / tile_size) * tile_size;
                    if (y - tile_size >= up_y and y + tile_size * 2 <= down_y) {
                        auto& room = br.addRoom({std::min(x0, x1), y-tile_size, (int)abs(x1-x0), tile_size*3}, LINE_CORRIDOR, 0.6f, 0);
                        d.linkRoom.push_back({i, {&room}});
                        continue;
                    }
                }

                Vector2i left_top{
                        (int)std::min(first.x, second.x),
                        (int)std::min(first.y, second.y),
                };
                Vector2i right_bottom{
                        (int)std::max(first.x, second.x),
                        (int)std::max(first.y, second.y),
                };
                left_top.x = int(left_top.x / tile_size - 1) * tile_size;
                left_top.y = int(left_top.y / tile_size - 1) * tile_size;
                right_bottom.x = int(right_bottom.x / tile_size + 2) * tile_size;
                right_bottom.y = int(right_bottom.y / tile_size + 2) * tile_size;

                int corridor_size = tile_size * 3;
                RectI rect[4] = {
                        {
                            // 上
                            left_top.x,
                            left_top.y,
                            right_bottom.x - corridor_size - left_top.x,
                            corridor_size,
                        },
                        {
                            // 右
                            right_bottom.x - corridor_size,
                            left_top.y,
                            corridor_size,
                            right_bottom.y - corridor_size - left_top.y,
                        },
                        {
                            // 下
                            left_top.x + corridor_size,
                            right_bottom.y - corridor_size,
                            right_bottom.x - corridor_size - left_top.x,
                            corridor_size,
                        },
                        {
                            // 左
                            left_top.x,
                            left_top.y + corridor_size,
                            corridor_size,
                            right_bottom.y - corridor_size - left_top.y,
                        },
                };
                std::vector<Room*> room_list;
                for (int i = 0; i < 4; ++i) {
                    auto& room = br.addRoom(rect[i], TURNING_CORRIDOR, 0.8f, 0);
                    room_list.push_back(&room);
                }
                d.linkRoom.push_back({i, room_list});
            }
            d.window->performLayout();
        }

        class check_line_corridor {
        public:
            check_line_corridor(Data& d, QuadTree<Room*>& qt):d(d), qt(qt) {}
            void check(QuadTree<Room*>::SquareList& ret, Corridor& c) {
                auto& room = c.rooms.back();
                qt.retrieve(ret, room->r);
                int main_room_count = 0;
                for (auto& r : ret) {
                    if (r->type == MAIN_ROOM and isCollision(r->r, room->r)) {
                        main_room_count++;
                    }
                }
                if (main_room_count >= 3) {
                    // 放弃当前走廊
                    d.invalidEdge[c.id] = true;
                    return;
                }
                for (auto& r : ret) {
                    if (r->type == MAIN_ROOM or r->type == LINE_CORRIDOR or r->type == TURNING_CORRIDOR) {
                        continue;
                    }
                    if (isCollision(r->r, room->r)) {
                        r->type = PASS_ROOM;
                        d.passRoom[r->id] = r;
                    }
                }
            }
        private:
            Data& d;
            QuadTree<Room*>& qt;
        };

        class turning_corridor {
        public:
            typedef std::shared_ptr<turning_corridor> Ptr;
            typedef std::vector<Ptr> Array;
        public:
            static Array Split(Corridor& c, Data& d, QuadTree<Room*>& qt) {
                Array ret;
                auto rooms = split_corridor(c, d);
                for (int i = 0; i < 2; ++i) {
                    auto p = std::make_shared<turning_corridor>(d, qt);
                    p->parse(rooms[i]);
                    ret.push_back(p);
                }
                return ret;
            }
            turning_corridor(Data& d, QuadTree<Room*>& qt):d(d), qt(qt) {}
            int getPassRoomSize() const {
                // 走廊上的小房间数量
                if (_passingMainRoom or _hasBlankRoad) {
                    // 经过主房间或者有空白路段，都算作无效走廊
                    return 0;
                }
                return _passRoomSize;
            }
            std::vector<Room*> const& getCorridor() const {
                return corridor;
            }
            std::vector<Room*> const& getPassRooms() const {
                return pass_rooms;
            }
        private:
            static std::vector<std::vector<Room*>> split_corridor(Corridor& c, Data& d) {
                assert(c.rooms.size() == 4 && "turning_corridor split_corridor error.");
                auto edge = d.edges[c.id];
                auto pt1 = &edge[0];
                auto pt2 = &edge[1];
                if (pt2->x < pt1->x) {
                    auto temp = pt1;
                    pt1 = pt2;
                    pt2 = temp;
                }
                std::vector<std::vector<Room*>> ret;
                if (pt1->y < pt2->y) {
                    ret.push_back({
                        c.rooms[0],
                        c.rooms[1],
                    });
                    ret.push_back({
                        c.rooms[2],
                        c.rooms[3],
                    });
                } else {
                    ret.push_back({
                        c.rooms[0],
                        c.rooms[3],
                    });
                    ret.push_back({
                        c.rooms[2],
                        c.rooms[1],
                    });
                }
                return ret;
            }
            void parse(std::vector<Room*> const& corridor) {
                _passRoomSize = 0;
                _passingMainRoom = false;
                _hasBlankRoad = false;
                int blank_road_count = 0;
                QuadTree<Room*>::SquareList ret;
                for (auto& room : corridor) {
                    bool has_blank_road = true;
                    int main_room_count = 0;
                    qt.retrieve(ret, room->r);
                    for (auto& r : ret) {
                        if (r->type == MAIN_ROOM) {
                            if (isCollision(r->r, room->r)) {
                                main_room_count++;
                                has_blank_road = false;
                            }
                            continue;
                        }
                        if (r->type == LINE_CORRIDOR or r->type == TURNING_CORRIDOR) {
                            continue;
                        }
                        if (isCollision(r->r, room->r)) {
                            // 小房间数量
                            _passRoomSize++;
                            has_blank_road = false;
                            pass_rooms.push_back(r);
                            continue;
                        }
                    }
                    if (main_room_count >= 2) {
                        _passingMainRoom = true;
                    }
                    if (has_blank_road) {
                        blank_road_count++;
                    }
                }
                if (blank_road_count >= 1) {
                    _hasBlankRoad = true;
                }
                this->corridor = corridor;
            }
        private:
            bool _passingMainRoom; // 是否经过主房间
            bool _hasBlankRoad; // 是否存在空白路段(走廊上没有小房间)
            int _passRoomSize;
            Data& d;
            QuadTree<Room*>& qt;
            std::vector<Room*> corridor;
            std::vector<Room*> pass_rooms;
        };

        class check_turning_corridor {
        public:
            check_turning_corridor(Data& d, QuadTree<Room*>& qt):d(d), qt(qt) {}
            void check(QuadTree<Room*>::SquareList& ret, Corridor& c) {
                auto paths = turning_corridor::Split(c, d, qt);
                assert(paths.size() == 2 && "check_turning_corridor check error.");
                if (paths[0]->getPassRoomSize() == 0 and paths[1]->getPassRoomSize() == 0) {
                    d.invalidEdge[c.id] = true;
                    return;
                }
                auto p = paths[0]->getPassRoomSize() > paths[1]->getPassRoomSize() ? paths[0].get() : paths[1].get();
                for (auto& r : p->getPassRooms()) {
                    d.passRoom[r->id] = r;
                }
                c.rooms = p->getCorridor();
            }
        private:
            Data& d;
            QuadTree<Room*>& qt;
        };

        void check_corridor(Context& c) {
            auto& d = mge::to<Data>(&c.data());

            typedef QuadTree<Room*> MyQuadTree;
            MyQuadTree quadtree(0, {0, 0, d.window->size().x, d.window->size().y}, [](MyQuadTree::Square const& square){
                return square->r;
            });
            for (auto& room : d.rooms) {
                quadtree.insert(&room);
            }

            MyQuadTree::SquareList ret;
            for (auto& corridor : d.linkRoom) {
                ret.clear();
                if (corridor.rooms.size() == 0) {
                    // 房间紧挨没有走廊
                    continue;
                } else if (corridor.rooms.size() == 1) {
                    // 如果只有一条走廊
                    check_line_corridor clc(d, quadtree);
                    clc.check(ret, corridor);
                    continue;
                }
                // 处理拐角走廊
                check_turning_corridor ctc(d, quadtree);
                ctc.check(ret, corridor);
                continue;
            }

            std::map<int, bool> display;
            for (auto& r : d.mainRoom) {
                display[r->id] = true;
            }
            for (auto& corridor : d.linkRoom) {
                if (d.invalidEdge[corridor.id]) {
                    continue;
                }
                for (auto& r : corridor.rooms) {
                    display[r->id] = true;
                }
            }
            for (auto& kv : d.passRoom) {
                display[kv.second->id] = true;
            }
            for (auto& room : d.rooms) {
                if (not display[room.id]) {
                    room.hide();
                }
            }
            d.window->performLayout();
        }

        void make_center(Context& c) {
            auto& d = mge::to<Data>(&c.data());
            std::vector<Room*> list;
            list.reserve(d.mainRoom.size() + d.passRoom.size() + d.linkRoom.size());
            for (auto& room : d.mainRoom) {
                list.push_back(room);
            }
            for (auto& kv : d.passRoom) {
                list.push_back(kv.second);
            }
            for (auto& corridor : d.linkRoom) {
                for (auto& room : corridor.rooms) {
                    list.push_back(room);
                }
            }
            Vector2i min((d.window->size() * 0.5f).to<int>()), max;
            for (int i = 0; i < list.size(); ++i) {
                auto& r = list[i]->r;
                if (r.x <= min.x) {
                    min.x = r.x;
                }
                if (r.x + r.w >= max.x) {
                    max.x = r.x + r.w;
                }
                if (r.y <= min.y) {
                    min.y = r.y;
                }
                if (r.y + r.h >= max.y) {
                    max.y = r.y + r.h;
                }
            }
            for (int i = 0; i < list.size(); ++i) {
                auto room = list[i];
                auto& r = room->r;
                r.x -= abs(min.x);
                r.y -= abs(min.y);
                room->modify();
            }
            for (auto& edge : d.edges) {
                for (auto& pt : edge) {
                    pt.x -= abs(min.x);
                    pt.y -= abs(min.y);
                }
            }
            d.window->setSize(max.x-min.x, max.y-min.y);
            d.window->performLayout();
        }

        void save_map(Context& c) {
            auto& d = mge::to<Data>(&c.data());

            // 写入钢铁砖块
            auto grid_size = d.window->size().to<int>() / d.tile_size;
            d.grid.resize(grid_size, TILE_STEEL);

            // 写入走廊
            for (auto& c : d.linkRoom) {
                for (auto& room : c.rooms) {
                    RectI rect{
                        room->r.x / d.tile_size,
                        room->r.y / d.tile_size,
                        room->r.w / d.tile_size,
                        room->r.h / d.tile_size,
                    };
                    for (int y = rect.y; y < rect.y + rect.h; ++y) {
                        for (int x = rect.x; x < rect.x + rect.w; ++x) {
                            d.grid.set(x, y, TILE_NONE);
                        }
                    }
                }
            }

            // 写入小房间
            for (auto& kv : d.passRoom) {
                auto room = kv.second;
                RectI rect{
                    room->r.x / d.tile_size,
                    room->r.y / d.tile_size,
                    room->r.w / d.tile_size,
                    room->r.h / d.tile_size,
                };
                for (int y = rect.y; y < rect.y + rect.h; ++y) {
                    for (int x = rect.x; x < rect.x + rect.w; ++x) {
                        d.grid.set(x, y, TILE_NONE);
                    }
                }
            }

            // 写入主房间
            for (auto room : d.mainRoom) {
                RectI rect{
                    room->r.x / d.tile_size,
                    room->r.y / d.tile_size,
                    room->r.w / d.tile_size,
                    room->r.h / d.tile_size,
                };
                for (int y = rect.y; y < rect.y + rect.h; ++y) {
                    for (int x = rect.x; x < rect.x + rect.w; ++x) {
                        d.grid.set(x, y, TILE_NONE);
                    }
                }
            }
        }

        void save_start_end_room(Context& c) {
            auto& d = mge::to<Data>(&c.data());

            // 获取所有叶子节点
            std::map<int, int> count;
            for (auto& edge : d.edgeMiniPathGraph) {
                count[edge->start]++;
                count[edge->end]++;
            }
            std::vector<int> leaf;
            for (auto& kv : count) {
                if (kv.second == 1) {
                    leaf.push_back(kv.first);
                }
            }

            // 判断y值最大的叶子节点选为开始节点
            int y = 0;
            int remove_index = 0;
            for (int i = 0; i < leaf.size(); ++i) {
                auto index = leaf[i];
                auto& room = d.roomVertex[index];
                if (int(room.y) > y) {
                    y = room.y;
                    d.start_room = index;
                    remove_index = i;
                }
            }

            // 从列表移除开始房间
            leaf.erase(leaf.begin() + remove_index);

            // 对开始房间颜色标注
            d.roomVertex[d.start_room].room->color({255, 255, 0, 255});

            // 判断剩余节点和开始节点的距离，取最远的节点作为最终房间
            auto& distance = *d.path_finding;
            float length = 0.0f;
            distance.init();
            for (int i = 0; i < leaf.size(); ++i) {
                auto next = leaf[i];
                auto value = distance.get(d.start_room, next);
                if (value > length) {
                    length = value;
                    d.end_room = next;
                }
            }

            // 对结束房间颜色标注
            d.roomVertex[d.end_room].room->color({255, 255, 0, 255});
        }
    }
}
