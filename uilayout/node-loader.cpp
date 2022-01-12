//
// Created by baifeng on 2022/1/11.
//

#include "node-loader.h"
#include "layout-config.h"
#include "layout-reader.h"
#include "pystring.h"
#include <string.h>
#include <vector>
#include "common/widget.h"
#include "common/loadres.h"
#include "common/game.h"

namespace ui {

    typedef NodeLoader::Node Node;
    using namespace mge;

    NodeLoader::~NodeLoader() {}

    Node NodeLoader::loadNode(mge::Widget* parent, LayoutReader* reader) {
        return createNode(parent, reader);
    }

    void NodeLoader::onParseProperty(mge::Widget* node, mge::Widget* parent, LayoutReader* reader, const char* name, const char* value) {
        if (strcmp(name, "Position") == 0) {
            std::vector<std::string> ret;
            pystring::split(value, ret, ",");
            if (ret.size() == 2) {
                float x = 0.0f, y = 0.0f;
                auto screen_size = _game.screen().screen_size();
                if (ret[0].back() == '%') {
                    x = (atof(ret[0].c_str()) / 100.0f) * (parent ? parent->size().x : screen_size.x);
                } else {
                    x = atof(ret[0].c_str());
                }
                if (ret[1].back() == '%') {
                    y = (atof(ret[1].c_str()) / 100.0f) * (parent ? parent->size().y : screen_size.y);
                } else {
                    y = atof(ret[1].c_str());
                }
                node->setPosition(x, y);
            }
        } else if (strcmp(name, "Size") == 0) {
            std::vector<std::string> ret;
            pystring::split(value, ret, ",");
            if (ret.size() == 2) {
                float x = 0.0f, y = 0.0f;
                auto screen_size = _game.screen().screen_size();
                if (ret[0].back() == '%') {
                    x = (atof(ret[0].c_str()) / 100.0f) * (parent ? parent->size().x : screen_size.x);
                } else {
                    x = atof(ret[0].c_str());
                }
                if (ret[1].back() == '%') {
                    y = (atof(ret[1].c_str()) / 100.0f) * (parent ? parent->size().y : screen_size.y);
                } else {
                    y = atof(ret[1].c_str());
                }
                node->setSize(x, y);
            }
        } else if (strcmp(name, "Anchor") == 0) {
            std::vector<std::string> ret;
            pystring::split(value, ret, ",");
            if (ret.size() == 2) {
                node->setAnchor(atof(ret[0].c_str()), atof(ret[1].c_str()));
            }
        }
    }

    Node ImageWidgetLoader::createNode(mge::Widget* parent, LayoutReader* reader) {
        return Node(new mge::ImageWidget(nullptr));
    }

    void ImageWidgetLoader::onParseProperty(mge::Widget* node, mge::Widget* parent, LayoutReader* reader, const char* name, const char* value) {
        if (strcmp(name, "Source") == 0) {
            auto& config = reader->config();
            auto file_name = config.RootImagePath + value;
            node->to<mge::ImageWidget>()->setTexture(res::load_texture(file_name.c_str()));
        } else {
            NodeLoader::onParseProperty(node, parent, reader, name, value);
        }
    }

    Node WindowWidgetLoader::createNode(mge::Widget* parent, LayoutReader* reader) {
        return Node(new mge::WindowWidget);
    }

    Node WidgetLoader::createNode(mge::Widget* parent, LayoutReader* reader) {
        return Node(new mge::Widget);
    }
}