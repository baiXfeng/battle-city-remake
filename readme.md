# battle-city-remake
famicom game battle city remake with sdl2.

build for Linux and macOS:
```
mkdir build
cd build
cmake ..
make
```


build for Psvita:
```
mkdir psv/build
cd psv/build
cmake -DBUILD_PSV=ON ../..
make
```