//
// Created by baifeng on 2022/1/11.
//

#ifndef SDL2_UI_LAYOUT_READER_H
#define SDL2_UI_LAYOUT_READER_H

#include <memory>

namespace mge {
    class Widget;
}

namespace ui {
    class FileReader;
    class LoaderPool;
    class Reader {
        typedef std::shared_ptr<mge::Widget> Node;
    public:
        Reader(LoaderPool* loader_library, FileReader* r);
        virtual ~Reader();
    public:
        Node readNode(std::string const& fileName);
    private:
        LoaderPool* _loader;
        FileReader* _freader;
    };
}

#endif //SDL2_UI_LAYOUT_READER_H
