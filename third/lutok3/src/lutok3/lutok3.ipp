//#include "lutok3"

#include <iostream>

namespace lutok3 {
    
    template<> inline void State::push(bool value)
    {
        lua_pushboolean(state, (value) ? 1 : 0);
    }

    template<> inline void State::push(int value)
    {
        lua_pushinteger(state, static_cast<lua_Integer>(value));
    }

    template<> inline void State::push(long value)
    {
        lua_pushinteger(state, static_cast<lua_Integer>(value));
    }

    template<> inline void State::push(float value)
    {
        lua_pushnumber(state, static_cast<lua_Number>(value));
    }

    template<> inline void State::push(double value)
    {
        lua_pushnumber(state, static_cast<lua_Number>(value));
    }

    /*template<> inline void State::push(const std::string & value)
    {
        lua_pushlstring(state, value.c_str(), value.length());
    }*/

    template<> inline void State::push(const char * value)
    {
        lua_pushstring(state, value);
    }

    template<> inline void State::push(void * value)
    {
        lua_pushlightuserdata(state, value);
    }

    template<typename T>
    void State::pushObject(T && object)
    {
        BaseObjectHelper<T> * rawObject = new BaseObjectHelper<T>(std::move(object));
        void ** internalPointer = reinterpret_cast<void**>(newUserData(sizeof(rawObject)));
        *internalPointer = reinterpret_cast<void*>(rawObject);

        newTable();
        pushFunction(BaseObjectHelper<CFunction>::gcHelper);
        setField(-2, "__gc");

        setMetatable(-2);
    }

    template<typename T>
    void State::pushUnmanagedObject(T && object)
    {
        T * rawObject = new T(std::move(object));
        void ** internalPointer = reinterpret_cast<void**>(newUserData(sizeof(rawObject)));
        *internalPointer = reinterpret_cast<void*>(rawObject);

        newTable();
        pushFunction(BaseObjectHelper<CFunction>::unmanagedGcHelper);
        setField(-2, "__gc");

        setMetatable(-2);
    }

    template<> inline void State::push(State & value)
    {
        lua_pushthread(value.state);
    }

    template<typename T>
    auto State::convertToCString(T&& t) const
    {
        if constexpr (std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, std::string>::value)
        {
            return std::forward<T>(t).c_str();
        }
        else
        {
            return std::forward<T>(t);
        }
    }

    template<typename ... Args>
    std::string State::stringFormatInternal(const std::string& format, Args&& ... args) const
    {
#if defined(_WIN32) && defined(_MSC_VER)
        size_t size = snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args) ...) + 1;
#else
        size_t size = snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args) ...) + 1;
#endif
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, format.c_str(), args ...);
        return std::string(buf.get(), buf.get() + size - 1);
    }

    template<typename ... Args>
    std::string State::stringFormat(std::string fmt, Args&& ... args) const
    {
        return stringFormatInternal(fmt, convertToCString(std::forward<Args>(args))...);
    }

    template<typename ...Args>
    void State::push(const std::string & fmt, Args&& ... args)
    {
        std::string buffer{ stringFormat(fmt, std::forward<Args>(args)...) };

        lua_pushlstring(state, buffer.c_str(), buffer.length());
    }

    template<class T>
    State::BaseObjectHelper<T>::BaseObjectHelper(T && object)
    {
        objectStorage = std::make_shared<T>(object);
    }

    template<class T>
    State::BaseObjectHelper<T>::~BaseObjectHelper()
    {
    }
    
    template<class T>
    int State::BaseObjectHelper<T>::gc(State & state)
    {
        return 0;
    }

    template<class T>
    int State::BaseObjectHelper<T>::gcHelper(lua_State * internalState)
    {
        State tmpState{ internalState };
        if (tmpState.type(1) == Type::UserData)
        {
            void ** rawPointer = reinterpret_cast<void**>(static_cast<void*>(tmpState.get(1)));
            BaseObjectHelper<T> * objectPointer = (reinterpret_cast<BaseObjectHelper<T>*>(*rawPointer));
            objectPointer->gc(tmpState);
            delete objectPointer;
        }
        return 0;
    }

    template<class T>
    int State::BaseObjectHelper<T>::unmanagedGcHelper(lua_State * internalState)
    {
        State tmpState{ internalState };
        if (tmpState.type(1) == Type::UserData)
        {
            void ** rawPointer = reinterpret_cast<void**>(static_cast<void*>(tmpState.get(1)));
            T * objectPointer = (reinterpret_cast<T*>(*rawPointer));
            delete objectPointer;
        }
        return 0;
    }

    template<class T>
    std::shared_ptr<T> State::BaseObjectHelper<T>::getStorage() const
    {
        return objectStorage;
    }
}