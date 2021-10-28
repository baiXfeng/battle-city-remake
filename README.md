# battle-city-remake
Famicom game battle city remake with sdl2.

### Windows install depends:
- Install [MinGW](https://www.mingw-w64.org/downloads/)
- Install [SDL2-MinGw-dev](http://www.libsdl.org/download-2.0.php)
- Install [SDL2_image-MinGw-dev](http://www.libsdl.org/projects/SDL_image/)
- Install [SDL2_ttf-MinGw-dev](http://www.libsdl.org/projects/SDL_ttf/)
- Install [SDL2_mixer-MinGw-dev](http://www.libsdl.org/projects/SDL_mixer/)

### Build for Windows
- Install [CLion](https://www.jetbrains.com/clion/) - A cross-platform IDE for C and C++
- Run CLion and open battle-city-remake folder
- Build and Run


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
- Install [Vitasdk](https://vitasdk.org)

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