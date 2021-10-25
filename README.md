# battle-city-remake
Famicom game battle city remake with sdl2.

### Linux install depends:
```
sudo apt install libsdl2-dev libsdl2-mixer-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-gfx-dev
```


### macOS install depends:
```
brew install sdl2 sdl2_image sdl2_mixer sdl2_ttf sdl2_gfx
```


### Build for Linux and macOS:
```
mkdir build
cd build
cmake ..
make
```

### Psvita install depends:
```
goto https://vitasdk.org
install vitasdk
```

### Build for Psvita:
```
mkdir build
cd build
cmake -DBUILD_PSV=ON ..
make
```

## Inspired by

- [BattleCity](https://github.com/newagebegins/BattleCity) - JavaScript remake of Famicom "Battle City" game
- [cattle-bity](https://github.com/dogballs/cattle-bity) - Clone of Battle City (1985) in TypeScript