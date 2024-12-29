/**
 * @file TestOptionalOperations.cpp
 * @brief Test for validation the optional operations work as intended.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#include <catch2/catch_test_macros.hpp>

#if RETROLIB_WITH_MODULES
import std;
import RetroLib;
#else
#include "RetroLib/Optionals/OptionalOperations.h"
#endif

TEST_CASE("Can perform basic operations on optional values", "[optionals]") {
    SECTION("Can get the values out of an optional correctly") {
        std::optional<int> optional1;
        std::optional optional2 = 1;

        CHECK_FALSE(Retro::Optionals::HasValue(optional1));
        CHECK(Retro::Optionals::HasValue(optional2));

        CHECK_THROWS_AS(Retro::Optionals::GetValue(optional1), std::bad_optional_access);
        CHECK(Retro::Optionals::GetValue(optional2) == 1);
        CHECK(Retro::Optionals::Get(optional2) == 1);
    }

    SECTION("Can convert an optional to a reference wrapped value") {
        std::optional optional = 1;
        decltype(auto) wrapped = Retro::Optionals::MakeOptionalReference(optional);
        CHECK(optional.value() == wrapped.value());
        CHECK(&optional.value() == &wrapped.value().get());

        decltype(auto) wrapped2 = Retro::Optionals::MakeOptionalReference(wrapped);
        CHECK(&wrapped == &wrapped2);

        constexpr std::optional optional2 = 4;
        decltype(auto) wrapped3 = Retro::Optionals::MakeOptionalReference(optional2);
        CHECK(optional2.value() == wrapped3.value());
        CHECK(&optional2.value() == &wrapped3.value().get());

        decltype(auto) wrapped4 =
            Retro::Optionals::MakeOptionalReference(Retro::Optionals::MakeOptionalReference(optional2));
        CHECK(wrapped4.value() == wrapped3.value());

        std::optional<int> optional3;
        decltype(auto) wrapped5 = Retro::Optionals::MakeOptionalReference(optional3);
        CHECK_FALSE(wrapped5.has_value());
        decltype(auto) wrapped6 = Retro::Optionals::MakeOptionalReference(std::as_const(optional3));
        CHECK_FALSE(wrapped6.has_value());
    }
}