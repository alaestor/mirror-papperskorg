cmake -S. -B"build" -G"Visual Studio 17 2022" -DCMAKE_CXX_COMPILER=cl -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
