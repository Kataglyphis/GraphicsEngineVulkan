mkdir build ; cd build
cmake --list-presets=all ../
cmake --preset linux-debug ../
cmake --build .