
-- 最大关卡数
LEVEL_MAX = 25

-- 添加砖块: tile.type
local function _add_tile(tile_type, position)
    add_tile(tile_type, position.x * tile.size, position.y * tile.size)
end

-- 添加基地
function base(position)
    _add_tile(tile.base, position)
end

-- 添加砖块
function brick(position)
    _add_tile(tile.brick, position)
end

-- 添加大砖块(由四个砖块组成)
function bricks(position)
    brick(position)
    brick({x = position.x + 0.5, y = position.y})
    brick({x = position.x, y = position.y + 0.5})
    brick({x = position.x + 0.5, y = position.y + 0.5})
end

-- 一行两个砖块
function brick_right(position)
    brick(position)
    brick({x = position.x + 0.5, y = position.y})
end

-- 一列两个砖块
function brick_down(position)
    brick(position)
    brick({x = position.x, y = position.y + 0.5})
end

-- 添加钢砖
function steel(position)
    _add_tile(tile.steel, position)
end

-- 添加大钢砖(由四个钢砖组成)
function steels(position)
    steel(position)
    steel({x = position.x + 0.5, y = position.y})
    steel({x = position.x, y = position.y + 0.5})
    steel({x = position.x + 0.5, y = position.y + 0.5})
end

-- 一行两个钢砖
function steel_right(position)
    steel(position)
    steel({x = position.x + 0.5, y = position.y})
end

-- 一列两个钢砖
function steel_down(position)
    steel(position)
    steel({x = position.x, y = position.y + 0.5})
end

-- 添加水面
function waters(position)
    _add_tile(tile.waters, position)
end

-- 添加冰地砖
function ice_floors(position)
    _add_tile(tile.ice_floors, position)
end

-- 添加森林
function trees(position)
    _add_tile(tile.trees, position)
end

-- 添加敌人坦克: tank.type
function add_enemy(enemy_type)
    add_tank(enemy_type)
end

--===============================================================
local auto_tile_proc = {
    [1] = bricks,
    [2] = brick_right,
    [3] = function(position)
        brick_down({x = position.x + 0.5, y = position.y})
    end,
    [4] = function(position)
        brick_right({x = position.x, y = position.y + 0.5})
    end,
    [5] = brick_down,

    [11] = steels,
    [12] = steel_right,
    [13] = function(position)
        steel_down({x = position.x + 0.5, y = position.y})
    end,
    [14] = function(position)
        steel_right({x = position.x, y = position.y + 0.5})
    end,
    [15] = steel_down,

    [21] = waters,
    [22] = trees,
    [23] = ice_floors,
}

local function get_pos(index)
    return {
        x = index % 13,
        y = math.floor(index / 13),
    }
end

-- 自动添加砖块
function auto_tile(tiles)
    for i = 1, #tiles do
        local pos = get_pos(i-1)
        local value = tiles[i]
        local proc = auto_tile_proc[value]
        if proc ~= nil then
            proc(pos)
        end
    end
end

-- 自动添加基地
function auto_base()
    base({x = 6, y = 12})
    for i=1,3 do
        brick({x = 5.5, y = 11.5 + (i - 1) * 0.5})
        brick({x = 7, y = 11.5 + (i - 1) * 0.5})
    end
    brick_right({x = 6, y = 11.5})
end
