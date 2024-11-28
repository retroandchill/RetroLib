cmake_minimum_required(VERSION 3.29)
list(APPEND CMAKE_PROJECT_TOP_LEVEL_INCLUDES ${CMAKE_SOURCE_DIR}/CMake/conan_provider.cmake)
project(RetroLib)

option(RETROLIB_WITH_TESTS "Should this project be compiled with the tests?" ON)
option(RETROLIB_WITH_MODULES "Should this project be compiled with the modules?" ON)

if(RETROLIB_WITH_MODULES)
    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
endif()


set(CMAKE_CXX_STANDARD 20)
add_subdirectory(RetroLib)

if(RETROLIB_WITH_TESTS)
    add_subdirectory(Tests)
endif()
