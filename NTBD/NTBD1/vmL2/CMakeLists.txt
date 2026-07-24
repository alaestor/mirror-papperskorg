cmake_minimum_required(VERSION 3.1.2)
project(vml2 LANGUAGES CXX)

# Executable target settings
add_executable(main ${CMAKE_SOURCE_DIR}/src/main.cpp)
target_compile_features(main PRIVATE cxx_std_20)
target_link_libraries(main)

if(CMAKE_COMPILER_IS_GNUCXX OR MINGW OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	target_compile_options(main PRIVATE -Wall -Wextra -Wundef -Wnull-dereference -Wpedantic -pedantic-errors -Weffc++ -Wnoexcept -Wuninitialized -Wunused -Wunused-parameter -Winit-self -Wconversion -Wuseless-cast -Wextra-semi -Wsuggest-final-types -Wsuggest-final-methods -Wsuggest-override  -Wformat-signedness -Wno-format-zero-length -Wmissing-include-dirs -Wshift-overflow=2 -Walloc-zero -Walloca -Wsign-promo -Wconversion -Wduplicated-branches -Wduplicated-cond -Wfloat-equal -Wshadow -Wshadow=local -Wmultiple-inheritance -Wvirtual-inheritance -Wno-virtual-move-assign -Wunsafe-loop-optimizations -Wnormalized -Wpacked -Wredundant-decls -Wmismatched-tags -Wredundant-tags -Wctor-dtor-privacy -Wdeprecated-copy-dtor -Wstrict-null-sentinel -Wold-style-cast -Woverloaded-virtual -Wzero-as-null-pointer-constant -Wconditionally-supported -Werror=pedantic -Wwrite-strings -Wmultiple-inheritance -Wunused-const-variable=2 -Wdouble-promotion -Wpointer-arith -Wcast-align=strict -Wcast-qual -Wconversion -Wsign-conversion -Wimplicit-fallthrough=1 -Wmisleading-indentation -Wdangling-else -Wdate-time -Wformat=2 -Wformat-overflow=2 -Wformat-signedness -Wformat-truncation=2 -Wswitch-default -Wswitch-enum -Wstrict-overflow=5 -Wstringop-overflow=4 -Warray-bounds=2 -Wattribute-alias=2 -Wcatch-value=2 -Wplacement-new=2 -Wtrampolines -Winvalid-imported-macros -Winvalid-imported-macros)
	target_compile_options(main PRIVATE -fno-rtti -Ofast -fgcse-las -fgcse-sm -fdeclone-ctor-dtor -fdevirtualize-speculatively -fdevirtualize-at-ltrans -ftree-loop-im -fivopts -ftree-loop-ivcanon -fira-hoist-pressure -fsched-pressure -fsched-spec-load -flto=auto -s -ffat-lto-objects -fno-enforce-eh-specs -fstrict-enums)
	target_compile_options(main PRIVATE $<$<CONFIG:RELEASE>:-DNDEBUG>)
	target_compile_options(main PRIVATE $<$<CONFIG:ASM>:-S>)
endif()

target_include_directories(main PUBLIC ${CMAKE_SOURCE_DIR}/libFGL/include)

# if(MINGW)
# 	target_link_libraries(main uuid)
# endif()

