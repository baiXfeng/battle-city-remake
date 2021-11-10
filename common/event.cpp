//
// Created by baifeng on 2021/10/2.
//

#include "event.h"
#include <algorithm>

mge_begin

void EventCenter::add(int event_id, Listener* listener) {
    this->remove(event_id, listener);
    this->_add(event_id, listener);
}

void EventCenter::_add(int event_id, Listener* listener) {
    _events[event_id].push_back(listener);
}

void EventCenter::remove(int event_id, Listener* listener) {
    auto& list = _events[event_id];
    auto iter = std::find(list.begin(), list.end(), listener);
    if (iter != list.end()) {
        list.erase(iter);
    }
}

void EventCenter::notify(Event const& e) {
    auto list = _events[e.Id()];
    for (auto& target : list) {
        target->onEvent(e);
    }
}

mge_end
