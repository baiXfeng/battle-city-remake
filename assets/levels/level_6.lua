
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
    0, 0, 0, 0, 0, 3, 0, 5,31,31, 0, 0, 0,
    0, 5,13, 0, 5, 0, 0, 0, 3,31, 5, 3,31,
    0, 5,13, 0, 5, 0, 1, 0, 3,31, 5, 3,31,
    0, 1, 0, 0, 1, 0,11, 0, 1,31, 0, 1,31,
    0, 0, 0, 3,12, 0, 1, 0, 2,15, 0,31,31,
    1, 1, 5, 0, 0,31, 1,31, 0, 0, 3, 1, 1,
    0, 0, 0, 0, 3,31,31,31, 5, 0, 0, 0, 0,
   11, 1, 1, 0, 2,31,31,31, 2, 3, 1, 1,11,
   12,12,12, 0, 4, 0,31, 0, 4, 0,12,12,12,
    0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 1, 5, 0, 0, 2, 0, 2, 0, 0, 3, 1,31,
    0, 0, 2, 0, 0, 0, 0, 0, 0, 0,31,31,31,
    0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 4,31,31,
})

auto_base()
