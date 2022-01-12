//
// Created by baifeng on 2022/1/12.
//

#ifndef SDL2_UI_LAYOUT_DOCUMENT_H
#define SDL2_UI_LAYOUT_DOCUMENT_H

#include "pugixml.hpp"

namespace ui {

    class Document {
    public:
        Document(pugi::xml_node const& n):node(n) {}
        pugi::xml_node& operator()() {
            return node;
        }
        void reset(pugi::xml_node const& n) {
            node = n;
        }
    private:
        pugi::xml_node node;
    };
}

#endif //SDL2_UI_LAYOUT_DOCUMENT_H
