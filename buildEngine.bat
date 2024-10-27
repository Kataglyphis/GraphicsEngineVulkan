mkdir build
cd build
cmake --preset x64-MSVC-Windows ../
cmake --build . --target GraphicsEngine --clean-first --verbose
pause