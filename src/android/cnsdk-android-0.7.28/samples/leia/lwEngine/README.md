# LightWeightEngine (LWE)

## How to build

### Windows

Prerequisites:
* CMake
* Visual Studio

Open a terminal and run the following commands from the root folder of the package to build and run the sample project:
```
cmake -DCMAKE_PREFIX_PATH=%CD% -S samples\leia\lwEngine -B build
cmake --build build --target install --config Release
bin\test_lwe_opengl.exe
```

### Android

See ../../README.md
