//
// Created by baifeng on 2021/10/10.
//

#ifndef SDL2_UI_LOG_H
#define SDL2_UI_LOG_H

namespace LOG {
    enum {
        none = 0,
        info,
        error,
        debug,
    };
}

#if defined(__vita__)
#include <debugnet.h>

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
#elif defined(__PSP__)
#include <pspdebug.h>

#define LOG_INIT() {    \
    pspDebugScreenInit(); \
}
#define LOG(format, args...) { \
    pspDebugScreenSetTextColor(0xFFFFFFFF); \
    pspDebugScreenPrintf(format, ##args);   \
}
#define LOG_FINI()
#else
#define LOG_INIT()
#define LOG(format, args...)
#define LOG_FINI()
#endif

#define ENABLE_LOG false

#if not ENABLE_LOG
#undef LOG_INIT
#undef LOG
#undef LOG_FINI
#define LOG_INIT()
#define LOG(format, args...)
#define LOG_FINI()
#endif

#endif //SDL2_UI_LOG_H
