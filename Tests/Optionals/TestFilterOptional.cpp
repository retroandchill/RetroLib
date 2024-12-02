/**
 * @file TestFilterOptional.cpp
 * @brief Test for the optional filter operation.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#include <catch2/catch_test_macros.hpp>

#if RETROLIB_WITH_MODULES
import std;
import RetroLib;
#else
#include "RetroLib/Functional/BindBack.h"
#include "RetroLib/Optionals/Filter.h"
#endif

TEST_CASE("Can filter an optional value", "[optionals]") {
    SECTION("Can filter on a constant functor") {
        constexpr auto is_even = [](int x) { return x % 2 == 0; };
        std::optional odd = 3;
        std::optional even = 4;
        static_assert(retro::optionals::Optional<decltype(odd)>);
        auto filtered_odd = retro::optionals::filter<is_even>(odd);
        CHECK_FALSE(filtered_odd.has_value());
        auto filtered_even = retro::optionals::filter<is_even>(even);
        CHECK(filtered_even.has_value());

        auto filtered_rvalue_odd = retro::optionals::filter<is_even>(std::optional(5));
        CHECK_FALSE(filtered_rvalue_odd.has_value());
        auto filtered_rvalue_even = retro::optionals::filter<is_even>(std::optional(6));
        CHECK(filtered_rvalue_even.has_value());
    }

    SECTION("Can filter on a constant functor") {
        constexpr auto greater_than = [](int x, int y) { return x > y; };
        std::optional odd = 3;
        std::optional even = 4;
        static_assert(retro::optionals::Optional<decltype(odd)>);
        auto filtered_odd = retro::optionals::filter(odd, retro::bind_back<greater_than>(4));
        CHECK_FALSE(filtered_odd.has_value());
        auto filtered_even = retro::optionals::filter(even, retro::bind_back<greater_than>(3));
        CHECK(filtered_even.has_value());

        auto filtered_rvalue_odd = retro::optionals::filter(std::optional(5), retro::bind_back<greater_than>(7));
        CHECK_FALSE(filtered_rvalue_odd.has_value());
        auto filtered_rvalue_even = retro::optionals::filter(std::optional(6), retro::bind_back<greater_than>(4));
        CHECK(filtered_rvalue_even.has_value());
    }
}