//
// Created by baifeng on 2021/11/8.
//

#ifndef SDL2_UI_OBSERVER_H
#define SDL2_UI_OBSERVER_H

#include <list>

template<class T>
class Observer {
public:
    typedef T View;
public:
    virtual ~Observer() {}
public:
    void add(View* view) {
        _views.push_back(view);
    }
    void remove(View* view) {
        auto iter = std::find(_views.begin(), _views.end(), view);
        if (iter != _views.end()) {
            _views.erase(iter);
        }
    }
    template<typename... Args>
    void notify(void (View::*method)(Args...), Args const&... args) {
        auto views = _views;
        for (auto& v : views) {
            if (v) {
                (v->*method)(args...);
            }
        }
    }
    void notify(void (View::*method)()) {
        auto views = _views;
        for (auto& v : views) {
            if (v) {
                (v->*method)();
            }
        }
    }
    void each(std::function<void(View* v)> catcher) {
        auto views = _views;
        for (auto& v : views) {
            catcher(v);
        }
    }
protected:
    std::list<View*> _views;
};

#endif //SDL2_UI_OBSERVER_H
