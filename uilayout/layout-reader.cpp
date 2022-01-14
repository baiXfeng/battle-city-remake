//
// Created by baifeng on 2022/1/11.
//

#include "layout-reader.h"
#include "file-reader.h"
#include "loader-pool.h"
#include "node-loader.h"
#include "layout-info.h"
#include "layout-variable-assigner.h"
#include "private/layout-document.h"
#include "pugixml.hpp"
#include "common/widget.h"
#include <assert.h>

namespace ui {

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
        _info.push_back(Info(new LayoutInfo(&doc)));
        assert(strcmp(doc().name(), "Layout") == 0 && "Reader::readNode fail<2>.");

        if (info().RootWidgetName.empty()) {
            // 未指派根视图名字
            doc.reset(doc().first_child());
            assert(!doc().empty() && strcmp(doc().name(), "XmlLayout") != 0 && "Reader::readNode fail<3>.");
        } else {
            // 使用指派的视图名字
            doc.reset(doc().find_node([this](pugi::xml_node const& n) {
                return strcmp(n.name(), info().RootWidgetName.c_str()) == 0;
            }));
            assert(!doc().empty() && strcmp(doc().name(), "XmlLayout") != 0 && "Reader::readNode fail<4>.");
        }

        auto node = readNode(parent, &doc, true);
        _info.pop_back();
        _owner.pop_back();
        return node;
    }

    LayoutReader::Node LayoutReader::readNode(mge::Widget* parent, Document* d, bool owner) {
        auto& doc = *d;
        auto loader = _loader->getLoader(doc().name());
        if (loader == nullptr) {
            printf("LayoutReader::readNode fail: <%s> is not exist.\n", doc().name());
            return nullptr;
        }
        if (strcmp(doc().name(), "XmlLayout") == 0) {
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

        // 保留根视图
        if (owner) {
            _owner.push_back(node.get());
        }

        this->parseProperties(loader, node.get(), parent, d);
        for (auto iter = doc().begin(); iter != doc().end(); iter++) {
            Document doc(*iter);
            auto child = readNode(node.get(), &doc);
            node->addChild(child);
        }

        // 完成通知
        auto notify = dynamic_cast<LayoutNodeListener*>(node.get());
        if (notify) {
            notify->onLayoutLoaded();
        }

        return node;
    }

    void LayoutReader::parseProperties(NodeLoader* loader, mge::Widget* node, mge::Widget* parent, Document* d) {
        auto& doc = *d;
        auto owner = this->owner();
        for (auto attr = doc().first_attribute(); not attr.empty(); attr = attr.next_attribute()) {
            if (strcmp(attr.name(), "Assign") == 0 and owner != node) {
                auto target = owner->to<LayoutVariableAssigner>();
                if (target) {
                    target->onAssignMember(owner, attr.value(), node);
                }
                continue;
            }
            loader->onParseProperty(node, parent, this, attr.name(), attr.value());
        }
    }

    void LayoutReader::assignMember(mge::Widget* target, const char* name, mge::Widget* node) {

    }

    LayoutInfo const& LayoutReader::info() const {
        return *_info.back().get();
    }

    mge::Widget* const LayoutReader::owner() const {
        return _owner.back();
    }
}