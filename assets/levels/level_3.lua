
--[[
1 - 砖块
2 - 砖块顶部两个
3 - 砖块右侧两个
4 - 砖块底部两个
5 - 砖块左侧两个

11 - 钢砖
12 - 钢砖顶部两个
13 - 钢砖右侧两个
14 - 钢砖底部两个
15 - 钢砖左侧两个

21 - 水
31 - 树
41 - 冰
]]--

auto_tile({
    0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
    0,31,31,31, 1, 0, 0, 0, 0, 0,14,14,14,
    1,31,31,31, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   31,31,31,31, 0, 0, 0, 1, 0, 1, 1, 1, 5,
   31,31,31,31, 1, 1, 1, 2, 0, 1, 0, 3, 0,
   31,31,31,31, 0, 0, 1, 0, 0, 0, 0, 3, 0,
    0,31, 0, 0, 0, 0,11,11,11, 0, 0,31, 0,
    0, 4, 0, 4, 0, 0, 0, 0, 0,31,31,31,31,
    1, 5, 3, 1, 5, 3, 2, 2, 2,31,31,31,31,
    0, 0, 0, 0, 0, 1, 0, 4, 4,31,31,31,31,
    1, 0, 0,15, 0, 0, 0, 2, 2,31,31,31, 0,
    1, 1, 0,15, 0, 0, 0, 0, 0,31,31,31, 0,
   11, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
})

auto_base()

local a = tank.a
local b = tank.b
local c = tank.c
local d = tank.d
local drop = true

auto_enemy({
    {a},
    {a},
    {a},
    {a, drop},
    {a},
    {a},
    {a},
    {a},
    {a},
    {a},
    {a, drop},
    {a},
    {a},
    {a},
    {b},
    {b},
    {b},
    {b, drop},
    {d},
    {d},
})