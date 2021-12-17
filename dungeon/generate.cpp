//
// Created by baifeng on 2021/12/17.
//

#include "generate.h"

namespace dungeon {

    Builder::Builder(mge::Data& d): _data(&d) {

    }

    Builder::Builder(mge::Data& d, Steps const& steps): _data(&d), _steps(steps) {

    }

    void Builder::setData(mge::Data& d) {
        _data = &d;
    }

    void Builder::addStep(Step const& step) {
        _steps.push_back(step);
    }

    void Builder::execute() {
        for (auto& step : _steps) {
            step(*_data);
        }
    }

    void Builder::clear() {
        _data = nullptr;
        _steps.clear();
    }

}