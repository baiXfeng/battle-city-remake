//
// Created by baifeng on 2022/1/12.
//

#include "layout-info.h"
#include "private/layout-document.h"

namespace ui {

#define ASSIGN_STRING(name, target) {       \
    auto attr = (*d)().attribute(name);     \
    if (!attr.empty()) {                    \
        target = attr.value();              \
    }                                       \
}

    LayoutInfo::LayoutInfo(Document* d) {
        ASSIGN_STRING("RootWidgetName", RootWidgetName);
    }
}