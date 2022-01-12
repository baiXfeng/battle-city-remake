//
// Created by baifeng on 2022/1/12.
//

#ifndef SDL2_UI_XML_LAYOUT_H
#define SDL2_UI_XML_LAYOUT_H

#include "types.h"

namespace ui {
    class FileReader;
    class LoaderPool;
    class LayoutReader;
}

mge_begin

class Widget;
class XmlLayout final {
    typedef std::shared_ptr<ui::FileReader> FileReader;
    typedef std::shared_ptr<ui::LoaderPool> LoaderPool;
    typedef std::shared_ptr<ui::LayoutReader> LayoutReader;
    typedef std::shared_ptr<Widget> WidgetPtr;
public:
    XmlLayout();
public:
    WidgetPtr readNode(std::string const& fileName);
    void setLoader(LoaderPool const& loader);
    void setFileReader(FileReader const& file_reader);
    LoaderPool& getLoaderPool();
    FileReader& getFileReader();
private:
    FileReader _fileReader;
    LoaderPool _loaderPool;
    LayoutReader _layoutReader;
};

mge_end

#endif //SDL2_UI_XML_LAYOUT_H
