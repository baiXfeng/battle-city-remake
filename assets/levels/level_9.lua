
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
    0, 0, 0, 1, 0, 0, 0, 0, 0,14,31, 0, 0,
    1, 0, 0, 0, 0, 0,14,31,13,11,15, 0, 1,
    0, 0, 0,14,31,13,11,15, 0,12,31, 0, 0,
    0, 0,13,11,15, 0,12,31, 0, 0, 0, 0, 0,
    0, 0, 0,12,31, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0,31,14,31, 0,31,14,31, 0, 0, 0,
   11, 1, 0,13,11,15, 0,13,11,15, 0, 1,11,
    0, 0, 0,31,12,31, 0,31,12,31, 0, 0, 0,
    0, 0, 0, 0,14, 0, 0, 0,14, 0, 0, 0, 0,
    1, 0, 0,13,11,15, 0,13,11,15, 0, 0, 1,
    1, 0, 0,31,12,31, 0,31,12,31, 0, 0, 1,
    0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0,
    0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0,
})

auto_base()
