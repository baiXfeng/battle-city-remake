
import "levels/level_api.lua" -- use import for dofile

--==============================================================================
-- 最大关卡数
LEVEL_MAX = 25
-- 初始生命数
DEFAULT_LIFE_MAX = 3

--==============================================================================
-- 玩家初始位置
PLAYER_DEFAULT_SPAWN_POSITIONS = {
    {x = tile.size * 4.5, y = tile.size * 12},
    {x = tile.size * 7.5, y = tile.size * 12},
}

-- 敌人初始位置
ENEMY_DEFAULT_SPAWN_POSITIONS = {
    {x = 0, y = 0},
    {x = tile.size * 6, y = 0},
    {x = tile.size * 12, y = 0},
}

--==============================================================================

-- 开局延迟时间
LEVEL_START_DELAY = 2
-- 玩家坦克出生无敌时长
PLAYER_SPAWN_SHIELD_DURATION = 3.5
-- 敌人坦克出生时间间隔
ENEMY_SPAWN_DELAY = 3

-- 奖励时间
POWERUP_DURATION = {
    FREEZE = 10,            -- 时停奖励时长
    SHIELD = 10,            -- 护罩奖励时长
    SHOVEL = 17,            -- 基地加固时长
}

--==============================================================================
local speedScale = 0.6

-- 子弹速度
local BulletSpeed = {
    Slow = 600 * speedScale,
    Fast = 900 * speedScale,
}

-- 移动速度
local MoveSpeed = {
    Slow = 120 * speedScale,
    Medium = 180 * speedScale,
    Fast = 240 * speedScale,
}

-- 射击延迟
local BulletRapidFireDelay = {
    Slow = 0.16,
    Fast = 0.04,
}

-- 子弹砖块伤害
local TankBulletWallDamage = {
    Low = 1,
    High = 2,
}

-- 己方坦克配置
PLAYER_TANK_ATTRIBUTES = {
    {
        bulletMaxCount = 1,
        bulletRapidFireDelay = BulletRapidFireDelay.Slow,
        bulletSpeed = BulletSpeed.Slow,
        bulletTankDamage = 1,
        bulletWallDamage = TankBulletWallDamage.Low,
        health = 1,
        moveSpeed = MoveSpeed.Medium,
    },
    {
        bulletMaxCount = 1,
        bulletRapidFireDelay = BulletRapidFireDelay.Slow,
        bulletSpeed = BulletSpeed.Fast,
        bulletTankDamage = 1,
        bulletWallDamage = TankBulletWallDamage.Low,
        health = 1,
        moveSpeed = MoveSpeed.Medium,
    },
    {
        bulletMaxCount = 2,
        bulletRapidFireDelay = BulletRapidFireDelay.Fast,
        bulletSpeed = BulletSpeed.Fast,
        bulletTankDamage = 1,
        bulletWallDamage = TankBulletWallDamage.Low,
        health = 1,
        moveSpeed = MoveSpeed.Medium,
    },
    {
        bulletMaxCount = 2,
        bulletRapidFireDelay = BulletRapidFireDelay.Fast,
        bulletSpeed = BulletSpeed.Fast,
        bulletTankDamage = 1,
        bulletWallDamage = TankBulletWallDamage.High,
        health = 1,
        moveSpeed = MoveSpeed.Medium,
    },
}

-- 敌方坦克配置
ENEMY_TANK_ATTRIBUTES = {
    {
        bulletMaxCount = 1,
        bulletRapidFireDelay = BulletRapidFireDelay.Slow,
        bulletSpeed = BulletSpeed.Slow,
        bulletTankDamage = 1,
        bulletWallDamage = TankBulletWallDamage.Low,
        health = 1,
        moveSpeed = MoveSpeed.Slow,
    },
    {
        bulletMaxCount = 1,
        bulletRapidFireDelay = BulletRapidFireDelay.Slow,
        bulletSpeed = BulletSpeed.Slow,
        bulletTankDamage = 1,
        bulletWallDamage = TankBulletWallDamage.Low,
        health = 1,
        moveSpeed = MoveSpeed.Fast,
    },
    {
        bulletMaxCount = 1,
        bulletRapidFireDelay = BulletRapidFireDelay.Slow,
        bulletSpeed = BulletSpeed.Fast,
        bulletTankDamage = 1,
        bulletWallDamage = TankBulletWallDamage.Low,
        health = 1,
        moveSpeed = MoveSpeed.Slow,
    },
    {
        bulletMaxCount = 1,
        bulletRapidFireDelay = BulletRapidFireDelay.Slow,
        bulletSpeed = BulletSpeed.Slow,
        bulletTankDamage = 1,
        bulletWallDamage = TankBulletWallDamage.Low,
        health = 4,
        moveSpeed = MoveSpeed.Slow,
    },
}

--==============================================================================
-- 开发者信息
AVATAR = "go-mascot"
INDIENOVA = "indienova.com/u/baifeng"
GITHUB = "github.com/baiXfeng"
QQ_GROUP = "QQ Group: 26410463"
DISCORD = "discord.gg/JerKYhDShD"
