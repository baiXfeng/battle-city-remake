//
// Created by baifeng on 2022/1/19.
//

#ifndef SDL2_UI_DATA_H
#define SDL2_UI_DATA_H

#include "macro.h"

mge_begin

    class Data {
    public:
        virtual ~Data() {}
    };

    template<class T>
    class DataPack : public Data {
    public:
        template<typename... Args>
        DataPack(Args const&... args):_data(args...) {}
        DataPack(T const& data):_data(data) {}
        T& data() {
            return _data;
        }
        T const& data() const {
            return _data;
        }
    private:
        T _data;
    };

    template<typename T>
    T& to(Data* d) {
        auto p = dynamic_cast<T*>(d);
        assert(p && "to type error.");
        return *p;
    }

    template<typename T>
    T* to_p(Data* d) {
        return dynamic_cast<T*>(d);
    }

mge_end

#endif //SDL2_UI_DATA_H
