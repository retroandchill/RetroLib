/**
 * @file RangesTerminalClosureTest.cpp
 * @brief Tests for range terminal pipe operations
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#include <catch2/catch_test_macros.hpp>
#include <map>
#include <set>

#if RETROLIB_WITH_MODULES
import std;
import RetroLib;
#else
#include "RetroLib/Ranges/Algorithm/ForEach.h"
#include "RetroLib/Ranges/Algorithm/Reduce.h"
#include "RetroLib/Ranges/Algorithm/To.h"
#include "RetroLib/Utils/Operators.h"

#include <array>
#include <vector>
#endif

TEST_CASE("Ranges can be converted into a collection type", "[ranges]") {
    static constexpr std::array values = {1, 2, 3, 4, 5};
    SECTION("Convert to a like range type") {
        auto vectored = values | retro::ranges::to<std::vector>();
        CHECK(vectored == std::vector({1, 2, 3, 4, 5}));
        CHECK(retro::ranges::container_capacity(vectored) == 5);
        CHECK(retro::ranges::container_max_size(vectored) == vectored.max_size());
    }

    SECTION("Can convert between unlike, but compatible, range types") {
        auto vectored = values | retro::ranges::to<std::vector<double>>();
        CHECK(vectored == std::vector({1.0, 2.0, 3.0, 4.0, 5.0}));
        CHECK(retro::ranges::container_capacity(vectored) == 5);
        CHECK(retro::ranges::container_max_size(vectored) == vectored.max_size());
    }

    SECTION("Can convert a vector into a set") {
        static constexpr std::array duplicated = {1, 1, 2, 2, 2, 2, 3, 3, 3, 3};
        auto as_set = duplicated | retro::ranges::to<std::set>();
        CHECK(as_set == std::set({1, 2, 3}));
    }

    SECTION("Can convert a range into a map using pairs") {
        static constexpr std::array pairs = {std::make_pair(1, 2), std::make_pair(3, 4)};
        auto as_map = pairs | retro::ranges::to<std::map<int, int>>();
        CHECK(as_map == std::map<int, int>({{1, 2}, {3, 4}}));
    }
}

TEST_CASE("Can iterate over a range using a functor", "[ranges]") {
    static constexpr std::array values = {1, 2, 3, 4, 5};
    SECTION("Can iterate over the values of a collection") {
        std::vector<int> vectored;
        values | retro::ranges::for_each([&vectored](int value) { vectored.push_back(value); });
        CHECK(vectored == std::vector({1, 2, 3, 4, 5}));

        constexpr auto add_to_vector = [](int value, std::vector<int> &vector) { vector.push_back(value); };
        vectored.clear();
        values | retro::ranges::for_each<add_to_vector>(std::ref(vectored));
        CHECK(vectored == std::vector({1, 2, 3, 4, 5}));
    }

    SECTION("Can iterate over a range of pairs using a two arg functor") {
        static constexpr std::array pairs = {std::make_pair(1, 2), std::make_pair(3, 4)};
        std::map<int, int> as_map;
        pairs | retro::ranges::for_each([&as_map](int key, int value) { as_map[key] = value; });
        CHECK(as_map == std::map<int, int>({{1, 2}, {3, 4}}));
    }
}

TEST_CASE("Can reduce a range to a single value", "[ranges]") {
    static constexpr std::array values = {1, 2, 3, 4, 5};
    SECTION("Can reduce a range to a single value using a runtime binding") {
        auto result = values | retro::ranges::reduce(0, retro::add);
        CHECK(result == 15);
    }

    SECTION("Can reduce a range to a single value using a constexpr binding") {
        auto result = values | retro::ranges::reduce<retro::add>(0);
        CHECK(result == 15);
    }
}