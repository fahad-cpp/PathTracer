CONFIG=Release
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE="$CONFIG" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --config "$CONFIG" --parallel
build/PathTracer
