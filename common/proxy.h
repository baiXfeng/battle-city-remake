//
// Created by baifeng on 2021/10/4.
//

#ifndef SDL2_UI_PROXY_H
#define SDL2_UI_PROXY_H

#include <unordered_map>
#include <memory>
#include "types.h"

mge_begin

template<typename T>
class Proxy {
public:
    typedef std::shared_ptr<Data> DataPtr;
    typedef std::unordered_map<T, DataPtr> DataCache;
public:
    virtual ~Proxy() {}
public:
    bool has(T const& key) const {
        return _data.find(key) != _data.end();
    }
    template<typename DataType, typename... Args>
    void set(T const& key, Args const&... args) {
        _data[key] = DataPtr(new DataPack<DataType>(args...));
    }
    template<typename DataType>
    DataType& get(T const& key) {
        auto dp = static_cast<DataPack<DataType>*>(_data[key].get());
        return dp->data();
    }
    template<typename DataType>
    DataType const& get(T const& key) const {
        auto iter = _data.find(key);
        if (iter == _data.end()) {
            return nullptr;
        }
        auto dp = static_cast<DataPack<DataType>*>(iter->second.get());
        return dp->data();
    }
    template<typename DataType, typename... Args>
    DataType& force_get(T const& key, Args const&... args) {
        if (!has(key)) {
            set<DataType>(key, args...);
        }
        return get<DataType>(key);
    }
    void remove(T const& key) {
        _data.erase(key);
    }
    void clear() {
        _data.clear();
    }
protected:
    DataCache _data;
};

mge_end

#endif //SDL2_UI_PROXY_H
