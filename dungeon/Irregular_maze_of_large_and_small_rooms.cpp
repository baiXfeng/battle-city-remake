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

        Data::Data():world(mge::b2WorldSugar::CreateWorld()), buildRetryCount(0), roomVertex(nullptr), window(nullptr), tile_size(8) {

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
                int const middle_room_num = 30;
                int const small_room_num = 60;

                GenRoom(d.grid.size().x * 0.55f, big_room_num, {10, 10}, {20, 20});
                queryMainRoom();
                if (d.mainRoom.size() < (big_room_num >> 1) and d.buildRetryCount <= 10) {
                    ++d.buildRetryCount;
                    build();
                    return;
                }
                GenRoom(d.grid.size().x * 0.6f, middle_room_num, {6, 6}, {16, 16});
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
                if (Random::get(0, 100) < 50) {
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
            int skin_count = 0;
            auto tile_size = d.tile_size;
            BuildRooms br(d);
            for (auto& edge : d.edgePathGraph) {
                auto& first = d.roomVertex[edge->start];
                auto& second = d.roomVertex[edge->end];

                auto left_x = std::max(first.room->r.x, second.room->r.x);
                auto right_x = std::min(first.room->r.x + first.room->r.w, second.room->r.x + second.room->r.w);
                if ((right_x - left_x) / tile_size >= 3) {
                    auto r0 = first.room->r.y < second.room->r.y ? first.room : second.room;
                    auto r1 = first.room->r.y > second.room->r.y ? first.room : second.room;

                    float x = (right_x - left_x) * 0.5f + left_x;
                    float y0 = r0->r.y + r0->r.h;
                    float y1 = r1->r.y;

                    if (int(y1-y0) == 0) {
                        ++skin_count;
                        continue;
                    }

                    x = int(x / tile_size) * tile_size;
                    if (x - tile_size >= left_x and x + tile_size * 2 <= right_x) {
                        auto& room = br.addRoom({x-tile_size, std::min(y0, y1), tile_size*3, (int)abs(y1-y0)}, LINE_CORRIDOR, 0.6f, 0);
                        d.linkRoom.push_back({&room});
                        continue;
                    }
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
                        ++skin_count;
                        continue;
                    }

                    y = int(y / tile_size) * tile_size;
                    if (y - tile_size >= up_y and y + tile_size * 2 <= down_y) {
                        auto& room = br.addRoom({std::min(x0, x1), y-tile_size, (int)abs(x1-x0), tile_size*3}, LINE_CORRIDOR, 0.6f, 0);
                        d.linkRoom.push_back({&room});
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
                                left_top.x,
                                left_top.y,
                                right_bottom.x - corridor_size - left_top.x,
                                corridor_size,
                        },
                        {
                                right_bottom.x - corridor_size,
                                left_top.y,
                                corridor_size,
                                right_bottom.y - corridor_size - left_top.y,
                        },
                        {
                                left_top.x + corridor_size,
                                right_bottom.y - corridor_size,
                                right_bottom.x - corridor_size - left_top.x,
                                corridor_size,
                        },
                        {
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
                d.linkRoom.push_back(room_list);
            }

            typedef QuadTree<Room*> MyQuadTree;
            MyQuadTree quadtree(0, {0, 0, d.window->size().x, d.window->size().y}, [](MyQuadTree::Square const& square){
                return square->r;
            });
            for (auto& room : d.rooms) {
                quadtree.insert(&room);
            }

            MyQuadTree::SquareList ret;
            for (auto& arr : d.linkRoom) {
                ret.clear();
                for (auto& room : arr) {
                    quadtree.retrieve(ret, room->r);
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
            }

            std::map<int, bool> tag;
            for (auto& r : d.mainRoom) {
                tag[r->id] = true;
            }
            for (auto& arr : d.linkRoom) {
                for (auto& r : arr) {
                    tag[r->id] = true;
                }
            }
            for (auto& kv : d.passRoom) {
                tag[kv.second->id] = true;
            }
            for (auto& room : d.rooms) {
                if (not tag[room.id]) {
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
            for (auto& arr : d.linkRoom) {
                for (auto& room : arr) {
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
            d.window->setSize(max.x-min.x, max.y-min.y);
            d.window->performLayout();
        }
    }
}
