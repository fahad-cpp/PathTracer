PRESET=clang-release
rm -f build/$PRESET/PathTracer
cmake --preset $PRESET
cmake --build --preset $PRESET
cp build/$PRESET/compile_commands.json build/
build/$PRESET/PathTracer
