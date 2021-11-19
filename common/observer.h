//
// Created by baifeng on 2021/11/8.
//

#ifndef SDL2_UI_OBSERVER_H
#define SDL2_UI_OBSERVER_H

#include <list>
#include <algorithm>
#include <functional>
#include "types.h"

mge_begin

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

namespace __private__ {
    static int signal_func_id = 0;
}

template<typename T>
class Signal {
public:
    typedef std::function<T> observer_type;
private:
    class Slot {
        friend class Signal<T>;
    public:
        Slot():_id(0) {}
        Slot(observer_type const& obs):_obs(obs), _id(++__private__::signal_func_id) {}
        Slot(Slot const& slot) {
            *this = slot;
        }
        Slot& operator=(Slot const& slot) {
            _id = slot._id;
            _obs = slot._obs;
            return *this;
        }
        bool operator==(Slot const& slot) const {
            return _id == slot._id;
        }
    private:
        observer_type& get() {
            return _obs;
        }
    private:
        int _id;
        observer_type _obs;
    };
public:
    typedef Slot slot_type;
    typedef std::list<slot_type> oberser_list;
public:
    virtual ~Signal() {}
public:
    template<typename... Args>
    void operator()(Args const&... args) {
        auto list = _list;
        for (auto& obs : list) {
            obs.get()(args...);
        }
    }
    void operator()() {
        auto list = _list;
        for (auto& obs : list) {
            obs.get()();
        }
    }
    slot_type connect(observer_type const& obs) {
        _list.push_back(obs);
        return _list.back();
    }
    void disconnect(Slot const& target) {
        auto iter = std::find(_list.begin(), _list.end(), target);
        if (iter != _list.end()) {
            _list.erase(iter);
        }
    }
    void clear() {
        _list.clear();
    }
    bool empty() const {
        return _list.empty();
    }
    int size() const {
        return _list.size();
    }
protected:
    oberser_list _list;
};

mge_end

#endif //SDL2_UI_OBSERVER_H
