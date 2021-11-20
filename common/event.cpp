//
// Created by baifeng on 2021/10/2.
//

#include "event.h"
#include <algorithm>

mge_begin

void EventCenter::add(int event_id, Listener* listener) {
    auto& obs = _events[event_id];
    obs.remove(listener);
    obs.add(listener);
}

void EventCenter::remove(int event_id, Listener* listener) {
    _events[event_id].remove(listener);
}

void EventCenter::notify(Event const& e) {
    _events[e.Id()].notify(&Listener::onEvent, e);
}

mge_end
