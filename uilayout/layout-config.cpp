//
// Created by baifeng on 2022/1/12.
//

#include "layout-config.h"
#include "private/layout-document.h"

namespace ui {

#define ASSIGN_STRING(name, target) {       \
    auto attr = (*d)().attribute(name);     \
    if (!attr.empty()) {                    \
        target = attr.value();              \
    }                                       \
}

    LayoutConfig::LayoutConfig(Document* d):RootWidget(nullptr) {
        ASSIGN_STRING("RootFontPath", RootFontPath);
        ASSIGN_STRING("RootImagePath", RootImagePath);
        ASSIGN_STRING("RootWidgetName", RootWidgetName);
    }
}