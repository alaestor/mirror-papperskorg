
# [libFGL](https://github.com/alaestor/libFGL) Getting Started

---

## Example Project

This example assumes that you have the [CMake](https://cmake.org/) build system, the [Conan](conan.io/) package manager, and a C++20 compiler (preferably GCC 12.1 or higher as it's officially compatible with libFGL). Also see the [libFGL Getting Started](https://alaestor.github.io/libFGL/page-fgl-getting_started.html) documentation.

> Note: This example tries to follow best practices where possible by using CMake targets and building "out-of-source". If you're unfamiliar with modern build practices, an examination of the `CMakeLists.txt` file may be useful.

1. Set `revisions_enabled=1` under the `[general]` section of your `conan.conf` file (`~/.conan/conan.conf` on Linux, and `%userprofile%/.conan/conan.conf` on Windows).

2. Add the libFGL conan remote by running
	```
	conan remote add fgl-conan https://fgl.jfrog.io/artifactory/api/conan/fgl-conan
	```

3. `git clone` or otherwise obtain a local copy of this example project.

4. Open a terminal and navigate to the root project directory (so that the current working directory is where `CMakeLists.txt` is located).

5. Run conan to retrieve the project dependencies (which is only `libFGL/latest` in this example). With these arguments, we tell it to generate files for `cmake` and output them to the `build/conan` subdirectory. This subdirectory is referenced in the `CMakeLists.txt` file.
	```
	conan install . -if "build/conan" -g "cmake"
	```
	> Note: If you add other conan dependencies to this example, you will probably need to add the `--build=missing` option.

6. Run CMake to generate build files using the `CMakeLists.txt` in current working directory `"."`, and output them to the `build` subdirectory.
	```
	cmake -H"." -B"build"
	```
	> Note: If you're using MinGW on windows, append the `-G"MinGW Makefiles"` [generator option](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html).

7. Run CMake to build the project using the generated files in the `build` subdirectory.
	```
	cmake --build build
	```

8. Finally, assuming everything went well, we can execute the example program.
	```
	./build/bin/main
	```
	> Note: `./build/bin/main.exe` on a windows system


---

Unix-like native
```
conan remote add fgl-conan https://fgl.jfrog.io/artifactory/api/conan/fgl-conan
conan install . -if "build/conan" -g "cmake"
cmake -H"." -B"build"
cmake --build build
./build/bin/main
```

Windows + MinGW
```
conan remote add fgl-conan https://fgl.jfrog.io/artifactory/api/conan/fgl-conan
conan install . -if "build/conan" -g "cmake"
cmake -H"." -B"build" -G"MinGW Makefiles
cmake --build build
./build/bin/main.exe
```
