#include "lutok3"

#include <iostream>

using namespace std;

int main(int argc, char ** argv, char ** env)
{
    lutok3::State lua;
    
    try
    {
        lua.openLibs();
        lua.loadString(R"LUA(
local args = {...}

print("Program input", ...)
print("Hello world from Lua!");

print("Input arguments:")
for k,v in ipairs(args) do
    print("\t", k, type(v), v)
end

local fn = args[#args]
local fn2 = fn

if (type(fn2)=="function") then
    print("A function returns:", fn2("1234"))
end

return 12345.6
        )LUA");

        lua.push(3.141f);
        lua.push(12345);
        lua.push("abc");
        lua.push("variable args %s %d", "abc", 12345);
        lua.pushFunction(
            [](lutok3::State & state) -> int
        {
            int nargs = state.getTop();
            std::string arg1 = state.get(1);
            state.push("Hello world string: " + arg1);
            return 1;
        });

        lua.pcall(5, 1);
        int64_t value = lua.get();
        cout << "Value returned: " << value;
    }
    catch (exception & e)
    {
        cerr << "Lua interpreter exception: " << e.what() << "\n";
    }
    return 0;
}