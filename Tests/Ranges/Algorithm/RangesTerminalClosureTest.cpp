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
#include "RetroLib/Ranges/Algorithm/AllOf.h"
#include "RetroLib/Ranges/Algorithm/AnyOf.h"
#include "RetroLib/Ranges/Algorithm/FindFirst.h"
#include "RetroLib/Ranges/Algorithm/ForEach.h"
#include "RetroLib/Ranges/Algorithm/NoneOf.h"
#include "RetroLib/Ranges/Algorithm/Reduce.h"
#include "RetroLib/Ranges/Algorithm/To.h"
#include "RetroLib/Ranges/Views/Filter.h"
#include "RetroLib/Utils/Operators.h"

#include <array>
#include <optional>
#include <vector>
#endif

TEST_CASE("Ranges can be converted into a collection type", "[ranges]") {
    static constexpr std::array values = {1, 2, 3, 4, 5};
    SECTION("Convert to a like range type") {
        auto vectored = values | Retro::Ranges::To<std::vector>();
        CHECK(vectored == std::vector({1, 2, 3, 4, 5}));
        CHECK(Retro::Ranges::container_capacity(vectored) == 5);
        CHECK(Retro::Ranges::container_max_size(vectored) == vectored.max_size());
    }

    SECTION("Can convert between unlike, but compatible, range types") {
        auto vectored = values | Retro::Ranges::To<std::vector<double>>();
        CHECK(vectored == std::vector({1.0, 2.0, 3.0, 4.0, 5.0}));
        CHECK(Retro::Ranges::container_capacity(vectored) == 5);
        CHECK(Retro::Ranges::container_max_size(vectored) == vectored.max_size());
    }

    SECTION("Can convert a vector into a set") {
        static constexpr std::array duplicated = {1, 1, 2, 2, 2, 2, 3, 3, 3, 3};
        auto as_set = duplicated | Retro::Ranges::To<std::set>();
        CHECK(as_set == std::set({1, 2, 3}));
    }

    SECTION("Can convert a range into a map using pairs") {
        static constexpr std::array pairs = {std::make_pair(1, 2), std::make_pair(3, 4)};
        auto as_map = pairs | Retro::Ranges::To<std::map<int, int>>();
        CHECK(as_map == std::map<int, int>({{1, 2}, {3, 4}}));
    }
}

TEST_CASE("Can iterate over a range using a functor", "[ranges]") {
    static constexpr std::array values = {1, 2, 3, 4, 5};
    SECTION("Can iterate over the values of a collection") {
        std::vector<int> vectored;
        values | Retro::Ranges::for_each([&vectored](int value) { vectored.push_back(value); });
        CHECK(vectored == std::vector({1, 2, 3, 4, 5}));

        constexpr auto add_to_vector = [](int value, std::vector<int> &vector) { vector.push_back(value); };
        vectored.clear();
        values | Retro::Ranges::for_each<add_to_vector>(std::ref(vectored));
        CHECK(vectored == std::vector({1, 2, 3, 4, 5}));
    }

    SECTION("Can iterate over a range of pairs using a two arg functor") {
        static constexpr std::array pairs = {std::make_pair(1, 2), std::make_pair(3, 4)};
        std::map<int, int> as_map;
        pairs | Retro::Ranges::for_each([&as_map](int key, int value) { as_map[key] = value; });
        CHECK(as_map == std::map<int, int>({{1, 2}, {3, 4}}));
    }
}

TEST_CASE("Can reduce a range to a single value", "[ranges]") {
    static constexpr std::array values = {1, 2, 3, 4, 5};
    SECTION("Can reduce a range to a single value using a runtime binding") {
        auto result = values | Retro::Ranges::reduce(0, Retro::Add);
        CHECK(result == 15);
    }

    SECTION("Can reduce a range to a single value using a constexpr binding") {
        auto result = values | Retro::Ranges::reduce<Retro::Add>(0);
        CHECK(result == 15);
    }
}

TEST_CASE("Can find the first value in a range", "[ranges]") {
    static constexpr std::array values = {1, 2, 3, 4, 5};
    constexpr auto is_multiple_of = [](int i, int j) { return i % j == 0; };
    SECTION("Can reduce using an inferred optional value") {
        auto valid_result = values | Retro::Ranges::Views::Filter<is_multiple_of>(3) | Retro::Ranges::FindFirst();
        CHECK(valid_result == 3);

        auto invalid_result = values | Retro::Ranges::Views::Filter<is_multiple_of>(10) | Retro::Ranges::FindFirst();
        CHECK_FALSE(invalid_result.has_value());
    }

    SECTION("Can reduce using an inferred template parameter") {
        auto valid_result =
            values | Retro::Ranges::Views::Filter<is_multiple_of>(3) | Retro::Ranges::FindFirst<std::optional>();
        REQUIRE(valid_result.has_value());
        CHECK(valid_result->get() == 3);

        auto invalid_result =
            values | Retro::Ranges::Views::Filter<is_multiple_of>(10) | Retro::Ranges::FindFirst<std::optional>();
        CHECK_FALSE(invalid_result.has_value());
    }

    SECTION("Can reduce using an explicit template parameter") {
        auto valid_result = values | Retro::Ranges::Views::Filter<is_multiple_of>(3) |
                            Retro::Ranges::find_first<Retro::Optional<int>>();
        CHECK(valid_result == 3);

        auto invalid_result =
            values | Retro::Ranges::Views::Filter<is_multiple_of>(10) | Retro::Ranges::find_first<std::optional<int>>();
        CHECK_FALSE(invalid_result.has_value());
    }
}

TEST_CASE("Can check a collection for all of, any of, and none of", "[ranges]") {
    static constexpr std::array values = {1, 2, 3, 4, 5};
    SECTION("Check the all of condition") {
        CHECK(values | Retro::Ranges::all_of(Retro::GreaterThan, 0));
        CHECK(values | Retro::Ranges::AllOf<Retro::GreaterThan>(0));
        CHECK_FALSE(values | Retro::Ranges::all_of(Retro::GreaterThan, 10));
        CHECK_FALSE(values | Retro::Ranges::AllOf<Retro::GreaterThan>(20));
    }

    SECTION("Check the none of condition") {
        CHECK_FALSE(values | Retro::Ranges::none_of(Retro::GreaterThan, 0));
        CHECK_FALSE(values | Retro::Ranges::none_of<Retro::GreaterThan>(0));
        CHECK(values | Retro::Ranges::none_of(Retro::GreaterThan, 10));
        CHECK(values | Retro::Ranges::none_of<Retro::GreaterThan>(20));
    }

    SECTION("Check the any of condition") {
        CHECK(values | Retro::Ranges::any_of(Retro::GreaterThan, 2));
        CHECK(values | Retro::Ranges::any_of<Retro::GreaterThan>(2));
        CHECK_FALSE(values | Retro::Ranges::any_of(Retro::GreaterThan, 10));
        CHECK_FALSE(values | Retro::Ranges::any_of<Retro::GreaterThan>(20));
    }
}