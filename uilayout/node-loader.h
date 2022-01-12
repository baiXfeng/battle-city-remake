//
// Created by baifeng on 2022/1/11.
//

#ifndef SDL2_UI_NODE_LOADER_H
#define SDL2_UI_NODE_LOADER_H

#include <memory>

namespace mge {
    class Widget;
}

namespace ui {

    class LayoutReader;
    class NodeLoader {
    public:
        typedef std::shared_ptr<mge::Widget> Node;
    public:
        virtual ~NodeLoader();
        virtual void onParseProperty(mge::Widget* node, mge::Widget* parent, LayoutReader* reader, const char* name, const char* value);
    public:
        Node loadNode(mge::Widget* parent, LayoutReader* reader);
    protected:
        virtual Node createNode(mge::Widget* parent, LayoutReader* reader) = 0;
    };

    class WidgetLoader : public NodeLoader {
        Node createNode(mge::Widget *parent, LayoutReader *reader) override;
    };

    class ImageWidgetLoader : public NodeLoader {
        Node createNode(mge::Widget* parent, LayoutReader* reader) override;
        void onParseProperty(mge::Widget* node, mge::Widget* parent, LayoutReader* reader, const char* name, const char* value) override;
    };

    class WindowWidgetLoader : public NodeLoader {
        Node createNode(mge::Widget *parent, LayoutReader *reader) override;
    };
}

#endif //SDL2_UI_NODE_LOADER_H
