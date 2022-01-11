//
// Created by baifeng on 2022/1/11.
//

#include "layout-reader.h"
#include "file-reader.h"
#include "pugixml.hpp"

namespace ui {

    class Document {
    public:
        pugi::xml_document& operator()() {
            return doc;
        }
    private:
        pugi::xml_document doc;
    };

    Reader::Reader(LoaderPool* loader_library, FileReader* r):_freader(r), _loader(loader_library) {

    }

    Reader::~Reader() {
        
    }

    Reader::Node Reader::readNode(std::string const& fileName) {
        Document doc;
        auto d = _freader->getData(fileName);
        doc().load_buffer(d->data(), d->size());
    }
}