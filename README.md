# CPU Path Tracer

Path Tracer that runs on CPU in C++.


## Latest Renders:<br>
![Glowing Ball](renders/glow.png)<br>
![Coloured Balls](renders/colored.png)<br>
![Reflective Ball](renders/reflective.png)


# Building from source

## Windows
- install a c++ compiler, cmake and ninja <br>
```batch
winget install cmake
winget install LLVM.LLVM
winget install Ninja-build.Ninja

```
- clone the repo. <br>
```batch
git clone --recursive https://github.com/fahad-cpp/PathTracer PathTracer
cd PathTracer

```
- build using cmake with ninja and run <br>
```batch
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
build\PathTracer.exe

```
## Linux
- install a c++ compiler, cmake and ninja using your package manager <br>
```bash
sudo pacman -S clang ninja cmake

```
- clone the repo
```bash
git clone --recursive https://github.com/fahad-cpp/PathTracer PathTracer
cd PathTracer

```
- build using cmake with ninja and run <br>
```bash
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/PathTracer

```

# Input
- `R` - Render again
- `P` - Print to PPM
- `ESC` - Exit
