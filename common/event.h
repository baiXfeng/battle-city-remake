//
// Created by baifeng on 2021/10/2.
//

#ifndef SDL2_UI_EVENT_H
#define SDL2_UI_EVENT_H

#include <list>
#include <unordered_map>
#include "observer.h"
#include "types.h"

mge_begin

class Event {
public:
    class Listener {
    public:
        virtual ~Listener() {}
        virtual void onEvent(Event const& e) {}
    };
public:
    Event(int id):_id(id) {}
    virtual ~Event() {}
    template<class T> T& data() {
        auto dp = static_cast<DataPack<T>*>(getData());
        return dp->data();
    }
    template<class T> T const& data() const {
        auto dp = static_cast<DataPack<T>*>((Data*)getData());
        return dp->data();
    }
    int Id() const {
        return _id;
    }
protected:
    virtual Data const* getData() const {
        return nullptr;
    }
private:
    int _id;
};

template<class T>
class EasyEvent : public Event {
public:
    typedef T Type;
public:
    EasyEvent(int id, Type const& data):Event(id), _data(data) {}
    template<typename... Args> EasyEvent(int id, Args const&... args):Event(id), _data(args...) {}
protected:
    Data const* getData() const override {
        return &_data;
    }
protected:
    DataPack<Type> _data;
};

class EventCenter {
public:
    typedef Event::Listener Listener;
    typedef std::unordered_map<int, Observer<Listener>> EventMap;
public:
    virtual ~EventCenter() {}
public:
    void add(int event_id, Listener* listener);
    void remove(int event_id, Listener* listener);
    void notify(Event const& e);
protected:
    EventMap _events;
};

mge_end

#endif //SDL2_UI_EVENT_H
