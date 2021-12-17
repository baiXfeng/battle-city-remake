//
// Created by baifeng on 2021/12/17.
//

#include "generate.h"
#include "effolkronium/random.hpp"

namespace dungeon {

    Context::Context(Builder* builder): _builder(builder) {

    }

    void Context::seek(uint32_t index) {
        _builder->_index = index;
        _builder->execute();
    }

    void Context::prev() {
        _builder->_index --;
        _builder->execute();
    }

    void Context::next() {
        _builder->_index ++;
        _builder->execute();
    }

    void Context::abort() {
        _builder->_index = _builder->_steps.size();
    }

    bool Context::is_abort() const {
        return _builder->_index >= _builder->_steps.size();
    }

    mge::Data& Context::data() {
        return *_builder->_data;
    }

    Builder::Builder(mge::Data& d):_index(0), _data(&d) {

    }

    Builder::Builder(mge::Data& d, Steps const& steps):_index(0), _data(&d), _steps(steps) {

    }

    void Builder::setData(mge::Data& d) {
        _data = &d;
    }

    void Builder::addStep(Step const& step) {
        _steps.push_back(step);
    }

    void Builder::execute() {
        Context c(this);
        while (_index < _steps.size()) {
            _steps[_index](c);
            _index++;
        }
    }

    void Builder::clear() {
        _data = nullptr;
        _steps.clear();
        _index = 0;
    }

    void Builder::reset() {
        _index = 0;
    }

    float roundm(float n, float m) {
        return floor((n + m - 1) / m) * m;
    }

    using Random = effolkronium::random_static;

    float random_normalized() {
        return Random::get(0.0f, 1.0f);
    }

    mge::Vector2f getRandomPointInCircle(float radius, mge::Vector2i const& size) {
        auto t = 2 * (3.14159265358979323846264338327950288) * random_normalized();
        auto r = sqrt(random_normalized());
        return {
                roundm(radius * r * cos(t), size.x),
                roundm(radius * r * sin(t), size.y)
        };
    }
}
