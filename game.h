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
#include "dungeon/mini_span_tree.h"
#include "lutok3.h"
#include <algorithm>

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

        auto edges = dungeon::create_edge_graph_with_delaunay_triangulate(idb, [&cloud](int start, int end){
            auto& first = cloud[start];
            auto& second = cloud[end];
            mge::Vector2f pt1{first.x, first.y};
            mge::Vector2f pt2{second.x, second.y};
            return pt1.distance(pt2) * 10;
        });
        auto result = dungeon::mini_span_tree_kruskal(edges, POINTS);
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
        //this->test_some();
        this->initCommand();
        _game.event().notify(mge::Event(EventID::BATTLE_CITY_INIT));
#ifdef WIN32
        _game.screen().push<StartView>();
#else
        _game.screen().push<RandomRoomView>();
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
    mge::Vector2i screenSize() override {
        return {1280, 960};
    }
};

#endif //SDL2_UI_GAME_H
