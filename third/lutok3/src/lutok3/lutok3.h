
#ifndef __LUTOK3__
#define __LUTOK3__

extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

#include <memory>
#include <exception>
#include <string>
//#include <filesystem>
#include <experimental/filesystem>
#include <cstdint>
#include <functional>
#include <type_traits>

namespace lutok3 {

#if _MSC_VER >= 1900 && _MSC_VER < 2000
    namespace fs = std::experimental::filesystem::v1;
#else
    namespace fs = std::experimental::filesystem;
#endif

    class State;
    enum class LuaStatus;

    using CFunction = std::function<int(State & state)>;
    using KFunction = std::function<int(State & state, LuaStatus status, lua_KContext context)>;
    using KPair = std::pair<lua_KContext, lua_KFunction>;

    using Index = int;

    enum class Type : int
    {
        None = LUA_TNONE,
        Nil	= LUA_TNIL,
        Boolean = LUA_TBOOLEAN,
        LightUserData = LUA_TLIGHTUSERDATA,
        Number		= LUA_TNUMBER,
        String		= LUA_TSTRING,
        Table		= LUA_TTABLE,
        Function	= LUA_TFUNCTION,
        UserData	= LUA_TUSERDATA,
        Thread		= LUA_TTHREAD
    };
    
    enum class LuaStatus : int
    {
        Ok = LUA_OK,
        Yield = LUA_YIELD,
        ErrorRuntime = LUA_ERRRUN,
        ErrorSyntax = LUA_ERRSYNTAX,
        ErrorMemory = LUA_ERRMEM,
        ErrorGCMM = LUA_ERRGCMM,
        ErrorError = LUA_ERRERR
    };

    enum class Operator : int
    {
        Add = LUA_OPADD,
        Sub = LUA_OPSUB,
        Mul = LUA_OPMUL,
        Div = LUA_OPDIV,
        IDiv = LUA_OPIDIV,
        Mod = LUA_OPMOD,
        Pow = LUA_OPPOW,
        Unm = LUA_OPUNM,
        Not = LUA_OPBNOT,
        BAnd = LUA_OPBAND,
        BOr = LUA_OPBOR,
        BXor = LUA_OPBXOR,
        Shl = LUA_OPSHL,
        Shr = LUA_OPSHR
    };

    enum class CompareResult : int
    {
        Equals = LUA_OPEQ,
        LessThan = LUA_OPLT,
        LessOrEquals = LUA_OPLE
    };

    enum class GCType : int
    {
        Stop = LUA_GCSTOP,
        Restart = LUA_GCRESTART,
        Collect = LUA_GCCOLLECT,
        Count = LUA_GCCOUNT,
        CountB = LUA_GCCOUNTB,
        Step = LUA_GCSTEP,
        SetPause = LUA_GCSETPAUSE,
        SetStepMul = LUA_GCSETSTEPMUL,
        IsRunning = LUA_GCISRUNNING
    };

    class State
    {
    public:

        class ReturnValue
        {
        public:
            explicit ReturnValue(lua_State * state, Index index);

            operator std::string();
            operator bool();
            operator double();
            operator float();
            //operator long();
            operator int();
            operator int64_t();
            operator CFunction();
            operator const void *();
            operator void *();
            operator State();

            Type getType();
        private:
            lua_State * state;
            Index index;
        };

        template<class T>
        class BaseObjectHelper
        {
        public:
            BaseObjectHelper(T && object);
            virtual ~BaseObjectHelper();

            virtual int gc(State & state);
            std::shared_ptr<T> getStorage() const;

            static int gcHelper(lua_State * internalState);
            static int unmanagedGcHelper(lua_State * internalState);
        protected:
            std::shared_ptr<T> objectStorage;
        };

        State();
        State(lua_State * luaState);
        ~State();

        void openLibs();

        void loadString(const std::string & code);
        void loadFile(const fs::path & filePath, const std::string & mode = "");
        void doString(const std::string & code);
        void doFile(const fs::path & filePath, const std::string & mode = "");

        void call(int nArgs = 0, int nResults = -1);
        void pcall(int nArgs = 0, int nResults = -1, Index handlerIndex = 0);

        ReturnValue get(Index index = -1);
        template<typename T> void push(T value);
        void pushFunction(CFunction && value);
        void pushManagedFunction(CFunction && value);
        void pushFunction(lua_CFunction value);
        void pushValue(Index index);
        void pushGlobalTable();
        void pushNil();
        template<typename ...Args>
            void push(const std::string & fmt, Args && ... args);
        template<typename T>
            void pushObject(T && object);
        template<typename T>
            void pushUnmanagedObject(T && object);
        void pop(size_t count = 1);
        void copy(Index srcElement, Index destElement);
        void replace(Index destIndex);
        void remove(Index index);
        void rotate(Index startingIndex, int offset);
        void insert(Index destIndex);

        Type getField(Index tableIndex, const std::string & key);
        Type getGlobal(const std::string & name);
        Type getI(Index tableIndex, int elementIndex);
        bool getMetatable(Index index);
        Type getMetatable(const std::string & name);
        Type getTable(Index index);
        Type getUserValue(Index index);

        void setField(Index index, const std::string & key);
        void setGlobal(const std::string & name);
        void setI(Index tableIndex, int elementIndex);
        void setMetatable(Index index);
        void setMetatable(const std::string & name);
        void setTable(Index index);
        void setUserValue(Index index);

        Type type(Index index);
        std::string typeName(Type typeId);

        void newTable();
        void newTable(int nArrayElements, int nOtherElements = 0);
        void * newUserData(size_t size);
        bool newMetatable(const std::string & name);

        int absIndex(Index index);
        int upvalueIndex(Index index);

        void arith(Operator luaOperator);
        CompareResult compare(Index leftElement, Index rightElement, Operator luaOperator);
        void concat(int nElements);
        void len(Index index);
        bool next(Index index);

        LuaStatus getStatus();
        Index getTop();
        void setTop(Index index);
        int checkStack(int nSlots);
        int error();
        int error(const char * fmt, ...);
        int gc(GCType type, int data);

        void xmove(State & destState, int nElements);

    private:
        lua_State * state;
        bool stateOwner = true;

        template<typename T>
        auto convertToCString(T&& t) const;

        template<typename ... Args>
        std::string stringFormatInternal(const std::string& format, Args&& ... args) const;

        template<typename ... Args>
        std::string stringFormat(std::string fmt, Args&& ... args) const;

        static int luaCFunctionWrapper(lua_State * internalState);
        static int luaUnmanagedCFunctionWrapper(lua_State * internalState);
    };

    std::ostream & operator<<(std::ostream & outputStream, State::ReturnValue & rightValue);
    std::string operator+(std::string & leftString, State::ReturnValue & rightValue);
    std::string operator+(const char * leftString, State::ReturnValue & rightValue);
}

#include "lutok3.ipp"

#endif