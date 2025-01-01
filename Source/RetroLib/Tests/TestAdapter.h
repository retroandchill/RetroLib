/**
 * @file TestAdapter.h
 * @brief TODO: Fill me out
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#ifdef __UNREAL__
#include "Tests/TestHarnessAdapter.h"
#else
#include <catch2/catch_test_macros.hpp>

#define TEST_CASE_NAMED(Name, ...) TEST_CASE(__VA_ARGS__)
#endif