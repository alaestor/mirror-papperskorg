cmake_minimum_required(VERSION 3.20)
project(libfgl_getting_started LANGUAGES CXX)

# Conan setup, including as system headers
set(CONAN_SYSTEM_INCLUDES ON)
include(${CMAKE_SOURCE_DIR}/build/conan/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# Executable target settings
add_executable(main ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_compile_features(main PRIVATE cxx_std_20)
target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(main CONAN_PKG::libFGL)
# This isn't needed if you're running GCC >= 12.1
target_compile_definitions(main PRIVATE FGL_ACKNOWLEDGE_INCOMPATIBLE_ENVIRONMENT)
