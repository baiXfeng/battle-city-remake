//
// Created by baifeng on 2021/10/10.
//

#ifndef SDL2_UI_LOG_H
#define SDL2_UI_LOG_H

#ifdef __vita__
#include <debugnet.h>
#endif

namespace LOG {
    enum {
        none = 0,
        info,
        error,
        debug,
    };
}

#ifdef __vita__
// console: socat udp-recv:18194 stdout
#define LOG_INIT() {    \
    debugNetInit("192.168.1.22", 18194, LOG::debug); \
}
#define LOG(format, args...) {  \
    debugNetPrintf(LOG::debug, format, ##args);    \
}
#define LOG_FINI() {    \
    debugNetFinish();   \
}
#else
#define LOG_INIT()
#define LOG(format, args...)
#define LOG_FINI()
#endif

#endif //SDL2_UI_LOG_H
