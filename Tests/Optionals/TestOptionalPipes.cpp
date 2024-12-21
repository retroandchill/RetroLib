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
#include "RetroLib/Optionals/Transform.h"
#include "RetroLib/TypeTraits.h"
#endif

TEST_CASE("Can filter an optional value", "[optionals]") {
    SECTION("Can filter on a constant functor") {
        constexpr auto is_even = [](int x, int y) { return x % y == 0; };
        std::optional odd = 3;
        std::optional even = 4;
        std::optional<int> empty;
        auto filtered_odd = odd | retro::optionals::filter<is_even>(2);
        CHECK_FALSE(filtered_odd.has_value());
        auto filtered_even = even | retro::optionals::filter<is_even>(2);
        CHECK(filtered_even.has_value());
        auto filtered_empty = empty | retro::optionals::filter<is_even>(2);
        CHECK_FALSE(filtered_empty.has_value());

        auto filtered_rvalue_odd = std::optional(5) | retro::optionals::filter<is_even>(2);
        CHECK_FALSE(filtered_rvalue_odd.has_value());
        auto filtered_rvalue_even = std::optional(6) | retro::optionals::filter<is_even>(2);
        CHECK(filtered_rvalue_even.has_value());
        auto filtered_empty_rvalue = std::optional<int>() | retro::optionals::filter<is_even>(2);
        CHECK_FALSE(filtered_empty_rvalue.has_value());
    }

    SECTION("Can filter on a runtime functor") {
        auto greater_than = [](int x, int y) { return x > y; };
        std::optional odd = 3;
        std::optional even = 4;
        std::optional<int> empty;
        static_assert(retro::optionals::OptionalType<decltype(odd)>);
        auto filtered_odd = odd | retro::optionals::filter(greater_than, 4);
        CHECK_FALSE(filtered_odd.has_value());
        auto filtered_even = even | retro::optionals::filter(greater_than, 3);
        CHECK(filtered_even.has_value());
        auto filtered_empty = empty | retro::optionals::filter(greater_than, 3);
        CHECK_FALSE(filtered_empty.has_value());

        auto filtered_rvalue_odd = std::optional(5) | retro::optionals::filter(greater_than, 7);
        CHECK_FALSE(filtered_rvalue_odd.has_value());
        auto filtered_rvalue_even = std::optional(6) | retro::optionals::filter(greater_than, 4);
        CHECK(filtered_rvalue_even.has_value());
        auto filtered_rvalue_empty = std::optional<int>() | retro::optionals::filter(greater_than, 4);
        CHECK_FALSE(filtered_rvalue_empty.has_value());
    }
}

TEST_CASE("Can transform an optional between various types", "[optionals]") {
    constexpr auto multiply_value = [](int x, int y) { return x * y; };
    SECTION("Can transform using a constant functor") {
        std::optional value = 3;
        auto transformed = value | retro::optionals::transform<multiply_value>(2);
        CHECK(transformed.has_value());
        CHECK(transformed.value() == 6);

        std::optional<int> empty_opt;
        auto empty_transformed = empty_opt | retro::optionals::transform<multiply_value>(2);
        CHECK_FALSE(empty_transformed.has_value());
    }

    SECTION("Can transform using a runtime defined functor") {
        std::optional value = 6;
        auto transformed = value | retro::optionals::transform(multiply_value, 5);
        CHECK(transformed.has_value());
        CHECK(transformed.value() == 30);
    }

    SECTION("Can transform a value returning a reference to hold said reference") {
        std::array values = {1, 2, 3, 4, 5};
        std::optional index = 4;
        auto transformed = index | retro::optionals::transform([&values](int i) -> decltype(auto) { return values[i]; });
        CHECK(transformed.has_value());
        CHECK(transformed.value() == 5);
    }

    SECTION("Can transform a value returning a pointer to hold a reference to that pointer") {
        std::array values = {1, 2, 3, 4, 5};
        auto transformer = [&values](int i) {
            if (values.size() > i) {
                return &values[i];
            }

            return nullptr;
        };
        retro::Optional index = 2;
        auto transformed = index | retro::optionals::transform(transformer);
        CHECK(transformed.has_value());
        CHECK(transformed.value() == 3);

        index = 6;
        auto second_pass = index | retro::optionals::transform(transformer);
        CHECK_FALSE(second_pass.has_value());
    }
}