/**
 * @file OptionalIteratorTest.cpp
 * @brief Test for the optional iterator semantics.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#include <catch2/catch_test_macros.hpp>

#if RETROLIB_WITH_MODULES
import std;
import RetroLib;
#else
#include "RetroLib/Optionals/Optional.h"
#include "RetroLib/Ranges/Views/Concat.h"

#include <ranges>
#endif

TEST_CASE("Verify that we can iterate over optionals", "[optionals]") {
    SECTION("Can use a ranged for loop") {
        Retro::Optional value = 3;

        int sum = 0;
        for (auto i : value) {
            sum += i;
        }
        CHECK(sum == 3);
    }

    SECTION("Can use an iterator based for loop") {
        const Retro::Optional value = 3;
        static_assert(std::ranges::sized_range<Retro::Optional<int>>);

        int sum = 0;
        for (auto it = value.begin(); it != value.end(); it++) {
            sum += *it;
        }
        CHECK(sum == 3);
    }

    SECTION("Can use as part of a range pipe") {
        std::vector<Retro::Optional<int>> values = {1, std::nullopt, 2, 3, std::nullopt, std::nullopt, std::nullopt};
        auto view = values | std::ranges::views::join;

        int sum = 0;
        for (auto i : view) {
            sum += i;
        }
        CHECK(sum == 6);
    }

    SECTION("Can be used to determine size") {
        auto view = Retro::Ranges::Views::Concat(Retro::Optional(1), Retro::Optional<int>(), Retro::Optional(2),
                                                 Retro::Optional<int>(), Retro::Optional<int>(), Retro::Optional(3));
        CHECK(view.size() == 3);
    }
}