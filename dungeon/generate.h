//
// Created by baifeng on 2021/12/17.
//

#ifndef SDL2_UI_GENERATE_H
#define SDL2_UI_GENERATE_H

#include "common/types.h"
#include <memory>

namespace dungeon {

    class Builder {
    public:
        typedef std::function<void(mge::Data&)> Step;
        typedef std::vector<Step> Steps;
    public:
        Builder(mge::Data& d);
        Builder(mge::Data& d, Steps const& steps);
    public:
        void setData(mge::Data& d);
        void addStep(Step const& step);
        void execute();
        void clear();
    private:
        mge::Data* _data;
        Steps _steps;
    };
};

#endif //SDL2_UI_GENERATE_H
