
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
   41,41,41,41,41,41,41,41,41,41,41,41,41,
   41,41,41,41,41,41,41,41,41,41,41,41,41,
   41,41,41, 1,41,41,41,41,41, 1,41,41,41,
   41, 1, 0, 1, 0, 1,41, 1, 0, 1, 0, 1,41,
   41, 2, 2, 1, 0, 0, 0, 0, 0, 1, 2, 2,41,
   41,41,41, 1, 4, 1,11, 1, 4, 1,41,41,41,
   11,41,41,41, 0,12, 0,12, 0,41,41,41,11,
   41,41,41,41, 0, 4, 0, 4, 0,41,41,41,41,
   41,41,41,41, 0, 1, 0, 1, 0,41,41,41,41,
   41,41,41, 1, 0, 0, 4, 0, 0, 1,41,41,41,
   41, 1,41, 1, 0,12,12,12, 0, 1,41, 1,41,
    0, 1, 4, 1, 0, 0, 0, 0, 0, 1, 4, 1, 0,
    0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0,
})

auto_base()

local a = tank.a
local b = tank.b
local c = tank.c
local d = tank.d
local drop = true

auto_enemy({
    {d},
    {d},
    {d},
    {d, drop},
    {d},
    {d},
    {d},
    {d},
    {a},
    {a},
    {a, drop},
    {a},
    {a},
    {a},
    {c},
    {c},
    {b},
    {b, drop},
    {b},
    {b},
})