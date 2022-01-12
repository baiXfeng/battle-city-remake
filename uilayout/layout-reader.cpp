//
// Created by baifeng on 2022/1/11.
//

#include "layout-reader.h"
#include "file-reader.h"
#include "loader-pool.h"
#include "node-loader.h"
#include "layout-config.h"
#include "pugixml.hpp"
#include "common/widget.h"
#include <assert.h>

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

    LayoutReader::LayoutReader(LoaderPool* loader_library, FileReader* r): _fileReader(r), _loader(loader_library) {

    }

    LayoutReader::~LayoutReader() {
        
    }

    void LayoutReader::setFileReader(FileReader* reader) {
        _fileReader = reader;
    }

    void LayoutReader::setLoaderPool(LoaderPool* loader) {
        _loader = loader;
    }

    LayoutReader::Node LayoutReader::readNode(std::string const& fileName, mge::Widget* parent) {
        pugi::xml_document xml;
        auto d = _fileReader->getData(fileName);
        assert(not d->empty() && "Reader::readNode fail<1>.");
        xml.load_buffer(d->data(), d->size());
        Document doc(xml.first_child());
        _config.push_back(Config(new LayoutConfig(&doc)));
        assert(strcmp(doc().name(), "Layout") == 0 && "Reader::readNode fail<2>.");
        doc.reset(doc().first_child());
        assert(strcmp(doc().name(), "XMLWidget") != 0 && "Reader::readNode fail<3>.");
        auto node = readNode(parent, &doc);
        _config.pop_back();
        return node;
    }

    LayoutReader::Node LayoutReader::readNode(mge::Widget* parent, Document* d) {
        auto& doc = *d;
        auto loader = _loader->getLoader(doc().name());
        if (loader == nullptr) {
            return nullptr;
        }
        if (strcmp(doc().name(), "XMLWidget") == 0) {
            // xml视图布局文件
            auto attr = doc().attribute("File");
            if (!attr.empty()) {
                auto node = readNode(attr.value(), parent);
                this->parseProperties(loader, node.get(), parent, d);
                return node;
            }
            return nullptr;
        }
        // 视图类
        auto node = loader->loadNode(parent, this);
        this->parseProperties(loader, node.get(), parent, d);
        for (auto iter = doc().begin(); iter != doc().end(); iter++) {
            Document doc(*iter);
            auto child = readNode(node.get(), &doc);
            node->addChild(child);
        }
        return node;
    }

    void LayoutReader::parseProperties(NodeLoader* loader, mge::Widget* node, mge::Widget* parent, Document* d) {
        auto& doc = *d;
        for (auto attr = doc().first_attribute(); not attr.empty(); attr = attr.next_attribute()) {
            loader->onParseProperty(node, parent, this, attr.name(), attr.value());
        }
    }

    LayoutConfig& LayoutReader::config() {
        return *_config.back().get();
    }
}