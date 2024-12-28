/**
 * @file GeneratorTest.cpp
 * @brief TODO: Fill me out
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#include <catch2/catch_test_macros.hpp>

#if RETROLIB_WITH_MODULES
import std;
import RetroLib;
#else
#include "RetroLib/Ranges/Views/Generator.h"

#include <vector>
#endif

namespace retro::ranges::testing {
    static Generator<int> generate_integers(int num) {
        for (int i = 0; i < num; i++) {
            co_yield i;
        }
    }
}

TEST_CASE("Can create a lazily evaluated generator", "[ranges]") {
    using namespace retro::ranges::testing;
    SECTION("Use a generator to loop through some numbers") {
        std::vector<int> numbers;
        for (int i : generate_integers(5)) {
            numbers.push_back(i);
        }
        CHECK(numbers == std::vector({0, 1, 2, 3, 4}));
    }

    SECTION("Can use a generator with a range pipe") {
        auto numbers = generate_integers(10) |
            retro::ranges::views::filter([](int value) { return value % 2 == 0; }) |
            retro::ranges::to<std::vector>();
        CHECK(numbers == std::vector({0, 2, 4, 6, 8}));
    }
}