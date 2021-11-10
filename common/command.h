//
// Created by baifeng on 2021/10/7.
//

#ifndef SDL2_UI_COMMAND_H
#define SDL2_UI_COMMAND_H

#include "event.h"
#include "assert.h"
#include <memory>

mge_begin

class Command {
public:
    typedef std::shared_ptr<Command> Ptr;
    class Creater {
    public:
        virtual ~Creater() {}
        virtual Ptr create() = 0;
    };
public:
    virtual ~Command() {}
    virtual void onEvent(Event const& e) = 0;
};

template<class T>
class CommandCreater : public Command::Creater {
public:
    Command::Ptr create() override {
        return Command::Ptr(new T);
    }
};

class CommandCenter;
class BaseCommandCenter : public Event::Listener {
public:
    void registerEvent(int eventId, CommandCenter* center);
    void unregisterEvent(int eventId, CommandCenter* center);
};

class CommandCenter : public BaseCommandCenter {
public:
    typedef std::shared_ptr<Command::Creater> Creater;
public:
    template<class T>
    void add(int eventId) {
        assert(_commandCreater.find(eventId) == _commandCreater.end() and "CommandCenter add fail.");
        _commandCreater[eventId] = Creater(new CommandCreater<T>);
        registerEvent(eventId, this);
    }
    void remove(int eventId) {
        _commandCreater.erase(eventId);
        unregisterEvent(eventId, this);
    }
    void clear() {
        for (auto& iter : _commandCreater) {
            unregisterEvent(iter.first, this);
        }
        _commandCreater.clear();
    }
protected:
    void onEvent(Event const& e) override {
        auto iter = _commandCreater.find(e.Id());
        if (iter != _commandCreater.end()) {
            iter->second->create()->onEvent(e);
        }
    }
protected:
    std::unordered_map<int, Creater> _commandCreater;
};

mge_end

#endif //SDL2_UI_COMMAND_H
