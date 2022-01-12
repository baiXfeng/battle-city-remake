//
// Created by baifeng on 2022/1/12.
//

#ifndef SDL2_UI_LAYOUT_CONFIG_H
#define SDL2_UI_LAYOUT_CONFIG_H

#include <string>

namespace ui {

    class Document;
    class LayoutConfig final {
    public:
        LayoutConfig(Document* d);
    public:
        std::string RootImagePath;
    };
}

#endif //SDL2_UI_LAYOUT_CONFIG_H
