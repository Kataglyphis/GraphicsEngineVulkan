mkdir build ; cd build
cmake --list-presets=all ../
cmake --preset linux-debug-clang ../
cmake --build . --clean-first 