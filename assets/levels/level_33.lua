
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
    0, 0, 0, 0,11, 0, 0, 0, 0,11, 0, 0, 0,
    0,11, 0, 0, 0,11, 0, 0,11,31,31, 0, 0,
    0, 0,11, 0, 0, 0, 0,11,31,14,15, 0, 0,
    0, 0, 0,11, 0,31,31,31,31,31, 0,13, 0,
    0,15, 0, 0,11,31,31,11,31, 0, 0,11, 0,
    0,12,15,31, 0,11,31,31,11, 0, 0,13, 0,
    0, 0,31,31,31,31,31, 0, 0,11, 0, 0, 0,
    0,14,15,31, 0,11,31, 0, 0, 0,11, 0, 0,
    0,31,31,31,11, 0,11, 0,14, 0, 0,11, 0,
   31,31,31,11, 0, 0, 0, 0,13, 0, 0, 0, 0,
    0, 0,11, 0, 0, 0, 0, 0, 0, 0, 0,13,11,
    0, 0, 0, 0, 0, 0, 0, 0, 0,14,15, 0, 0,
   15, 0,14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
})

auto_base()

local a = tank.a
local b = tank.b
local c = tank.c
local d = tank.d
local drop = true

auto_enemy({
    {b},
    {b},
    {b},
    {b, drop},
    {d},
    {d},
    {d},
    {d},
    {d},
    {d},
    {d, drop},
    {d},
    {c},
    {c},
    {c},
    {c},
    {b},
    {b, drop},
    {b},
    {b},
})