//
// Created by baifeng on 2022/1/11.
//

#ifndef SDL2_UI_LOADER_POOL_H
#define SDL2_UI_LOADER_POOL_H

#include <map>
#include <string>
#include <memory>

namespace ui {

    class NodeLoader;
    class LoaderPool {
        typedef std::shared_ptr<NodeLoader> Loader;
        typedef std::map<std::string, Loader> LoaderCache;
    public:
        virtual ~LoaderPool() {}
        void registerDefaultLoader();
        void addLoader(std::string const& className, Loader const& loader);
        void removeLoader(std::string const& className);
        NodeLoader* getLoader(std::string const& className);
        template<class T>
        void addLoader(std::string const& className) {
            addLoader(className, Loader(new T));
        }
    private:
        LoaderCache _cache;
    };
}

#endif //SDL2_UI_LOADER_POOL_H
