//
// Created by baifeng on 2022/1/12.
//

#ifndef SDL2_UI_LAYOUT_INFO_H
#define SDL2_UI_LAYOUT_INFO_H

#include <string>

namespace mge {
    class Widget;
}

namespace ui {

    class Document;
    class LayoutInfo final {
    public:
        LayoutInfo(Document* d);
    public:
        std::string RootWidgetName;
    };
}

#endif //SDL2_UI_LAYOUT_INFO_H
