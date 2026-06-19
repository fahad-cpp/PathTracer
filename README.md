# CPU Path Tracer

software path tracer in C++

## Latest Renders:<br>
![Path Traced Balls](renders/diffuse.png)<br>
![Glowing Ball](renders/glow.png)<br>
![Coloured Balls](renders/colored.png)


# Building from source

- install a c++ compiler, cmake and ninja <br>
- for windows
```batch
winget install cmake
winget install LLVM.LLVM
winget install Ninja-build.Ninja
```
- or for linux
```bash
sudo pacman -S clang ninja cmake
```

- clone the repo and generate ninja files
```batch
git clone https://github.com/fahad-cpp/PathTracer PathTracer
cd PathTracer
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
```
- build the project
```batch
cmake --build build --config Release
```
- run it
```bash
./build/PathTracer
```
OR
```batch
build\PathTracer.exe
```

# Input
- `R` - Render again
- `P` - Print to PPM
- `ESC` - Exit