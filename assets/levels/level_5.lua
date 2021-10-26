
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
    0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0,12,12,11, 0, 0,
   11, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    1, 0, 1, 1, 1, 0, 1, 1, 0,21,21, 0,21,
    2, 0, 0, 0, 2, 0, 0, 0, 0,21, 0, 0, 0,
    0, 0, 4, 0,21,21, 0,21,21,21, 0, 1, 1,
    1, 1, 0, 0,21, 1, 0, 1, 5, 0, 0, 0, 0,
    0, 0, 0, 0,21, 0, 0, 0, 0, 0,13,15, 0,
   21,21,21, 0,21, 0,11, 0, 1, 0,13, 0, 0,
    0, 0, 0, 4, 4, 0, 0, 0, 0, 0,13, 1, 1,
    0, 0, 0, 0, 1, 2, 2, 2, 1, 4, 0, 0, 0,
    1, 1, 2, 0, 0, 0, 0, 0, 0, 2, 1, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
})

auto_base()

local a = tank.a
local b = tank.b
local c = tank.c
local d = tank.d
local drop = true

auto_enemy({
    {c},
    {c},
    {c},
    {c, drop},
    {c},
    {d},
    {d},
    {a},
    {a},
    {a},
    {a, drop},
    {a},
    {a},
    {a},
    {a},
    {b},
    {b},
    {b, drop},
    {b},
    {b},
})