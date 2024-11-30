/**
 * @file RangesToTest.cpp
 * @brief Tested for ranges to.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#include <catch2/catch_test_macros.hpp>

#if RETROLIB_WITH_MODULES
import std;
import RetroLib;
#else
#include "RetroLib/Ranges/Algorithm/To.h"

#include <array>
#include <vector>
#endif

TEST_CASE("Ranges can be converted into a collection type") {
    static constexpr std::array values = {1, 2, 3, 4, 5};
    SECTION("Convert to a like range type") {
        auto vectored = values |
            retro::ranges::to<std::vector>();
        CHECK(vectored == std::vector({1, 2, 3, 4, 5}));
    }

    SECTION("Can convert between unlike, but compatible, range types") {
        auto vectored = values |
            retro::ranges::to<std::vector<double>>();
        CHECK(vectored == std::vector({1.0, 2.0, 3.0, 4.0, 5.0}));
    }
}