cmake_minimum_required(VERSION 3.29)
list(APPEND CMAKE_PROJECT_TOP_LEVEL_INCLUDES ${CMAKE_SOURCE_DIR}/CMake/conan_provider.cmake)
project(RetroLib)

option(RETROLIB_WITH_TESTS "Should this project be compiled with the tests?" ON)

set(CMAKE_CXX_STANDARD 20)
add_subdirectory(RetroLib)

if(RETROLIB_WITH_TESTS)
    add_subdirectory(Tests)
endif()
