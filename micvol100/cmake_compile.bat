@rem These are the commands to run to compile this example on Windows with cmake + gcc
@rem conan install . -if "build/conan" -g "cmake" --build=missing
cmake -H. -B"build" -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build