//
// Created by baifeng on 2021/10/7.
//

#include "command.h"
#include "game.h"

void CommandCenterBase::registerEvent(int eventId, CommandCenter* center) {
    _game.event().add(eventId, center);
}

void CommandCenterBase::unregisterEvent(int eventId, CommandCenter* center) {
    _game.event().remove(eventId, center);
}