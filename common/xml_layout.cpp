//
// Created by baifeng on 2022/1/12.
//

#include "xml_layout.h"
#include "uilayout/ui-layout.h"

mge_begin

XmlLayout::XmlLayout():_fileReader(new ui::DiskFileReader), _loaderPool(new ui::LoaderPool) {
    _loaderPool->registerDefaultLoader();
    _layoutReader.reset(new ui::LayoutReader(_loaderPool.get(), _fileReader.get()));
}

XmlLayout::WidgetPtr XmlLayout::readNode(std::string const& fileName) {
    return _layoutReader->readNode(fileName);
}

ui::LoaderPool& XmlLayout::loader() {
    return *_loaderPool.get();
}

void XmlLayout::setFileReader(FileReader const& file_reader) {
    _layoutReader->setFileReader((_fileReader = file_reader).get());
}

mge_end