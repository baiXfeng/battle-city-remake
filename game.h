//
// Created by baifeng on 2021/7/12.
//

#ifndef SDL2_UI_GAME_H
#define SDL2_UI_GAME_H

#include "common/game.h"
#include "common/log.h"
#include "common/command.h"
#include "common/widget.h"
#include "src/command.h"
#include "src/const.h"
#include "src/view.h"
#include "survival/world.h"
#include "survival/menu.h"
#include "delabella.h"
#include "lutok3.h"
#include <algorithm>

typedef struct Edge {
    int id;
    int start;
    int end;
    int weight;
    Edge():id(0), start(0), end(0), weight(0) {}
} Edge;

typedef std::vector<Edge> EdgeGraph;
typedef std::vector<Edge*> EdgeGraphNoCopy;
typedef std::vector<mge::Vector2i> UVArray;
typedef std::vector<int> WeightArray;
typedef std::vector<int> VertexArray;
typedef std::vector<VertexArray> VertexTree;

EdgeGraph create_edge_graph(UVArray const& lines, WeightArray const& weights) {
    assert(lines.size() == weights.size() && "create_edge_graph fail.");
    EdgeGraph edges;
    edges.resize(lines.size());
    for (int i = 0; i < edges.size(); ++i) {
        auto& edge = edges[i];
        auto& line = lines[i];
        edge.id = i + 1;
        edge.start = line.x;
        edge.end = line.y;
        edge.weight = weights[i];
    }
    return edges;
}

bool edge_compare(Edge const& e1, Edge const& e2) {
    return e1.weight < e2.weight;
}

bool kruskal_find_tree(int start, int end, VertexTree& tree) {
    int temp_start = -1;
    int temp_end = -1;
    for (int i = 0; i < tree.size(); ++i) {
        auto& array = tree[i];
        auto array_begin = array.begin();
        auto array_end = array.end();
        if (std::find(array_begin, array_end, start) != array_end) {
            temp_start = i;
        }
        if (std::find(array_begin, array_end, end) != array_end) {
            temp_end = i;
        }
    }
    if (temp_start != temp_end) {
        auto& end_array = tree[temp_end];
        auto& start_array = tree[temp_start];
        for (int i = 0; i < end_array.size(); ++i) {
            start_array.push_back(end_array[i]);
        }
        end_array.clear();
        return true;
    }
    return false;
}

EdgeGraphNoCopy mini_span_tree_kruskal(EdgeGraph& edges, int vertexCount) {
    EdgeGraphNoCopy ret;
    VertexTree tree(vertexCount);
    for (int i = 0; i < vertexCount; ++i) {
        tree[i].push_back(i);
    }
    ret.reserve(vertexCount - 1);
    std::sort(edges.begin(), edges.end(), edge_compare);
    for (int i = 0; i < edges.size(); ++i) {
        auto& edge = edges[i];
        if (kruskal_find_tree(edge.start, edge.end, tree)) {
            ret.push_back(&edge);
        }
    }
    return ret;
}

EdgeGraph create_edge_graph_with_delaunay_triangulate(IDelaBella* idb, std::function<double(int start, int end)> const& cost) {
    UVArray uvarr;
    WeightArray weights;
    std::map<int, bool> tag;
    int const verts = idb->GetNumOutputVerts();
    int const index_scale = idb->GetNumInputPoints() << 8;
    if (verts >= 1) {
        // 保存每条边
        const DelaBella_Triangle* dela = idb->GetFirstDelaunayTriangle();
        int tris = verts / 3;
        for (int i = 0; i < tris; i++) {
            auto v0 = dela->v[0];
            auto v1 = dela->v[1];
            auto v2 = dela->v[2];
            int key0 = std::min(v0->i, v1->i) * index_scale + std::max(v0->i, v1->i);
            if (not tag[key0]) {
                auto first = v0->i < v1->i ? v0 : v1;
                auto second = v1->i > v0->i ? v1 : v0;
                tag[key0] = true;
                uvarr.push_back({first->i, second->i});
            }
            int key1 = std::min(v1->i, v2->i) * index_scale + std::max(v1->i, v2->i);
            if (not tag[key1]) {
                auto first = v2->i < v1->i ? v2 : v1;
                auto second = v1->i > v2->i ? v1 : v2;
                tag[key1] = true;
                uvarr.push_back({first->i, second->i});
            }
            int key2 = std::min(v2->i, v0->i) * index_scale + std::max(v2->i, v0->i);
            if (not tag[key2]) {
                auto first = v0->i < v2->i ? v0 : v2;
                auto second = v2->i > v0->i ? v2 : v0;
                tag[key2] = true;
                uvarr.push_back({first->i, second->i});
            }
            dela = dela->next;
        }
        // 计算每条边的权重
        weights.reserve(uvarr.size());
        for (int i = 0; i < uvarr.size(); ++i) {
            auto& uv = uvarr[i];
            weights.push_back(cost(uv.x, uv.y));
        }
        // 生成连通图
        return create_edge_graph(uvarr, weights);
    }
    return {};
}

class MyGame : public mge::Game::App {
private:
    lutok3::State* _state;
public:
    MyGame():_state(&_game.force_get<lutok3::State>("lua")) {}
    void test_some() {
        int POINTS = 6;
        struct MyPoint {
            float x;
            float y;
        };
        MyPoint* cloud = new MyPoint[POINTS];

        srand(36341);

        printf("顶点: %d个\n", POINTS);
        for (int i = 0; i < POINTS; i++) {
            cloud[i].x = rand() % 200;
            cloud[i].y = rand() % 200;
            printf("NO.%d x = %d, y = %d\n", i + 1, int(cloud[i].x), int(cloud[i].y));
        }

        printf("\n");

        IDelaBella* idb = IDelaBella::Create();
        idb->Triangulate(POINTS, &cloud->x, &cloud->y, sizeof(MyPoint));

        auto edges = create_edge_graph_with_delaunay_triangulate(idb, [&cloud](int start, int end){
            auto& first = cloud[start];
            auto& second = cloud[end];
            mge::Vector2f pt1{first.x, first.y};
            mge::Vector2f pt2{second.x, second.y};
            return pt1.distance(pt2) * 10;
        });
        auto result = mini_span_tree_kruskal(edges, POINTS);
        printf("最小生成树: %d条边(总边数%d条)\n", (int)result.size(), int(edges.size()));
        for (int i = 0; i < result.size(); ++i) {
            auto& edge = *result[i];
            auto v0 = &cloud[edge.start];
            auto v1 = &cloud[edge.end];
            auto first = v0;
            auto second = v1;
            printf("NO.%d(%d-%d-%d) x0 = %d, y0 = %d, x1 = %d, y1 = %d\n", i+1, edge.start + 1, edge.end + 1, edge.weight, int(first->x), int(first->y), int(second->x), int(second->y));
        }

        delete[] cloud;
        idb->Destroy();
        exit(0);
    }
    void initCommand() {
        _game.command().add<BattleModeInitCommand>(EventID::BATTLE_CITY_INIT);
        _game.command().add<GameOverCommand>(EventID::GAME_OVER_ANIMATION);
        _game.command().add<PauseGameCommand>(EventID::PAUSE_GAME);
        _game.command().add<ResumeGameCommand>(EventID::RESUME_GAME);
        _game.command().add<PlayerWinCommand>(EventID::PLAYER_WIN);
        _game.command().add<LoadResCommand>(EventID::LOAD_RES);
    }
    void init() override {
        LOG_INIT();
        this->test_some();
        this->initCommand();
        _game.event().notify(mge::Event(EventID::BATTLE_CITY_INIT));
#ifdef WIN32
        _game.screen().push<StartView>();
#else
        _game.screen().push<LogoView>();
#endif

#if defined(__PSP__)
        SDL_RenderSetScale(_game.renderer(), 0.5f, 0.5f);
#endif
    }
    void update(float delta) override {
        _game.screen().update(delta);
    }
    void render(SDL_Renderer* renderer) override {
        _game.screen().render(renderer);
    }
    void fini() override {
        _game.screen().popAll();
        _game.remove("lua");
        LOG_FINI();
    }
};

#endif //SDL2_UI_GAME_H
