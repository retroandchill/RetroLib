/**
 * @file ConcatViewTest.cpp
 * @brief Test for the concat view implementation
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#include <catch2/catch_test_macros.hpp>

#if RETROLIB_WITH_MODULES
import std;
import RetroLib;
#else
#include "RetroLib/Ranges/Views/Concat.h"

#include <array>
#include <vector>
#endif

TEST_CASE("Can concatenate two unlike ranges into a single unified view", "[ranges]") {
    std::array range1 = {1, 2, 3, 4, 5};
    std::vector range2 = {6, 7, 8, 9, 10};

    SECTION("Can concatenate and use with a ranged for loop") {
        int sum = 0;
        for (decltype(auto) i : retro::ranges::views::concat(range1, range2)) {
            sum += i;
        }
        CHECK(sum == 55);
    }

    SECTION("Can use an iterator based view setup") {
        auto view = retro::ranges::views::concat(range1, range2);
        int sum = 0;
        for (auto it = view.begin(); it != view.end(); ++it) {
            sum += *it;
        }
        CHECK(sum == 55);
    }

    SECTION("Can concatenate and use with a range adaptor chain") {
        auto view_chain = retro::ranges::views::concat(range1, range2) |
                          retro::ranges::views::filter([](int i) { return i % 2 == 0; });
        int sum = 0;
        for (int i : view_chain) {
            sum += i;
        }
        CHECK(sum == 30);
    }

    SECTION("Can iterate using a value based for loop") {
        auto view = retro::ranges::views::concat(range1, range2);
        int sum = 0;
        for (size_t i = 0; i < view.size(); i++) {
            sum += view[i];
        }
        CHECK(sum == 55);
    }
}