//
// Created by baifeng on 2022/1/12.
//

#include "layout-config.h"
#include "private/layout-document.h"

namespace ui {

    LayoutConfig::LayoutConfig(Document* d) {
        {
            auto attr = (*d)().attribute("RootImagePath");
            if (!attr.empty()) {
                RootImagePath = attr.value();
            }
        }
    }
}