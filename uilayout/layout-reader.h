//
// Created by baifeng on 2022/1/11.
//

#ifndef SDL2_UI_LAYOUT_READER_H
#define SDL2_UI_LAYOUT_READER_H

#include <memory>
#include <vector>

namespace mge {
    class Widget;
}

namespace ui {
    class Document;
    class FileReader;
    class LoaderPool;
    class NodeLoader;
    class LayoutConfig;
    class LayoutReader {
        typedef std::shared_ptr<mge::Widget> Node;
        typedef std::shared_ptr<LayoutConfig> Config;
        typedef std::vector<Config> ConfigStack;
    public:
        LayoutReader(LoaderPool* loader_library, FileReader* r);
        virtual ~LayoutReader();
    public:
        Node readNode(std::string const& fileName, mge::Widget* parent = nullptr);
        void setFileReader(FileReader* reader);
        void setLoaderPool(LoaderPool* loader);
    public:
        LayoutConfig& config();
    private:
        Node readNode(mge::Widget* parent, Document* d);
        void parseProperties(NodeLoader* loader, mge::Widget* node, mge::Widget* parent, Document* d);
    private:
        LoaderPool* _loader;
        FileReader* _fileReader;
        ConfigStack _config;
    };
}

#endif //SDL2_UI_LAYOUT_READER_H
