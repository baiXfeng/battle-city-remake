//
// Created by baifeng on 2022/1/11.
//

#include "node-loader.h"
#include "layout-config.h"
#include "layout-reader.h"
#include "pystring.h"
#include <string.h>
#include <vector>
#include <algorithm>
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
        } else if (strcmp(name, "Scale") == 0) {
            std::vector<std::string> ret;
            pystring::split(value, ret, ",");
            if (ret.size() == 2) {
                node->setScale(atof(ret[0].c_str()), atof(ret[1].c_str()));
            }
        } else if (strcmp(name, "Rotation") == 0) {
            node->setRotation(atof(value));
        } else if (strcmp(name, "Name") == 0) {
            node->setName(value);
        }
    }

    Node ImageWidgetLoader::createNode(mge::Widget* parent, LayoutReader* reader) {
        return Node(new mge::ImageWidget(nullptr));
    }

    void ImageWidgetLoader::onParseProperty(mge::Widget* node, mge::Widget* parent, LayoutReader* reader, const char* name, const char* value) {
        if (strcmp(name, "Source") == 0) {
            auto& config = reader->config();
            auto file_name = config.RootImagePath + value;
            node->fast_to<mge::ImageWidget>()->setTexture(res::load_texture(file_name.c_str()));
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

    Node TTFLabelLoader::createNode(mge::Widget* parent, LayoutReader* reader) {
        return Node(new mge::TTFLabel);
    }

    SDL_Color getHexColor(const char* hex_text) {
        std::string text(hex_text);
        std::transform(text.begin(), text.end(), text.begin(), std::toupper);
        if (text.length() < 10 or (text[0] != '0' and text[1] != 'X')) {
            return {0, 0, 0, 255};
        }
        auto char2hex = [](char c) {
            if (c >= '0' and c <= '9') {
                return c - '0';
            } else if (c >= 'A' and c <= 'F') {
                return 10 + c - 'A';
            }
            return  0;
        };
        auto hex2int = [char2hex](char first, char second) {
            return Uint8(char2hex(first) * 0x10 + char2hex(second));
        };
        return {
            hex2int(text[2], text[3]),
            hex2int(text[4], text[5]),
            hex2int(text[6], text[7]),
            hex2int(text[8], text[9]),
        };
    }

    void TTFLabelLoader::onParseProperty(mge::Widget* node, mge::Widget* parent, LayoutReader* reader, const char* name, const char* value) {
        if (strcmp(name, "Font") == 0) {
            std::vector<std::string> ret;
            pystring::split(value, ret, ":");
            if (ret.size() == 2) {
                auto& config = reader->config();
                auto file_name = config.RootFontPath + ret[0];
                auto font = res::load_ttf_font(file_name, atoi(ret[1].c_str()));
                node->fast_to<TTFLabel>()->setFont(font);
            }
        } else if (strcmp(name, "Text") == 0) {
            node->fast_to<TTFLabel>()->setString(value);
        } else if (strcmp(name, "Color") == 0) {
            node->fast_to<TTFLabel>()->font()->setColor(getHexColor(value));
        } else {
            NodeLoader::onParseProperty(node, parent, reader, name, value);
        }
    }
}