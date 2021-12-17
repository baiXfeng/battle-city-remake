//
// Created by baifeng on 2021/12/17.
//

#ifndef SDL2_UI_GENERATE_H
#define SDL2_UI_GENERATE_H

#include "common/types.h"
#include <memory>

namespace dungeon {

    class Builder;
    class Context {
    public:
        Context(Builder* builder);
    public:
        void seek(uint32_t index);
        void prev();
        void next();
        void abort();
        bool is_abort() const;
        mge::Data& data();
    private:
        Builder* _builder;
    };
    class Builder {
        friend class Context;
    public:
        typedef std::function<void(Context&)> Step;
        typedef std::vector<Step> Steps;
    public:
        Builder(mge::Data& d);
        Builder(mge::Data& d, Steps const& steps);
    public:
        void setData(mge::Data& d);
        void addStep(Step const& step);
        void execute();
        void clear();
        void reset();
    private:
        uint32_t _index;
        mge::Data* _data;
        Steps _steps;
    };

    float roundm(float n, float m);
    float random_normalized();
    mge::Vector2f getRandomPointInCircle(float radius, mge::Vector2i const& size = {4, 4});
};

#endif //SDL2_UI_GENERATE_H
