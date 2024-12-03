/**
 * @file TestOptional.cpp
 * @brief Test for the custom optional type.
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

#include <string>
#endif

TEST_CASE("Can use the new optional type", "[optionals]") {
    SECTION("Can create and assign optionals") {
        retro::Optional<int> optional1;
        CHECK_FALSE(optional1.has_value());
        CHECK_THROWS_AS(optional1.value(), std::bad_optional_access);
        CHECK_THROWS_AS(std::as_const(optional1.value()), std::bad_optional_access);
        CHECK_THROWS_AS(std::move(optional1).value(), std::bad_optional_access);

        retro::Optional optional2 = 3;
        CHECK(optional2.has_value());
        CHECK(optional2 == 3);
        CHECK(optional2.value() == 3);
        CHECK(*optional2 == 3);

        retro::Optional<std::string> optional3 = "Hello";
        CHECK(optional3.has_value());
        CHECK(optional3 == "Hello");
        CHECK(std::as_const(optional3).value() == "Hello");
        CHECK(std::move(optional3).value() == "Hello");
        CHECK(*std::as_const(optional3) == "Hello");
        CHECK(*std::move(optional3) == "Hello");

        optional3.reset();
        CHECK_FALSE(optional3.has_value());

        optional3.emplace("New string");
        CHECK(optional3.has_value());
        CHECK(optional3 != "Hello");
        CHECK(optional3 == "New string");
        CHECK(optional3->length() == 10);
        CHECK(std::as_const(optional3)->length() == 10);

        auto optional4 = std::move(optional3);
        REQUIRE(optional3.has_value());
        CHECK(optional3->empty());

        auto optional5 = optional4;
        REQUIRE(optional4.has_value());
        CHECK(optional4 == "New string");

        optional3.reset();
        CHECK_FALSE(optional3.has_value());

        auto optional6 = optional3;
        CHECK_FALSE(optional6.has_value());
        auto optional7 = std::move(optional3);
        CHECK_FALSE(optional3.has_value());

        optional4 = optional3;
        CHECK_FALSE(optional3.has_value());
        optional5 = std::move(optional3);
        CHECK_FALSE(optional3.has_value());
    }

    SECTION("Can compare optionals between each other") {
        retro::Optional<int> optional1;
        retro::Optional optional2 = 3;
        CHECK_FALSE(optional1 == optional2);
        CHECK(optional1 < optional2);
        CHECK(optional2 > optional1);

        retro::Optional optional3 = 7.0;
        CHECK(optional1 != optional3);
        CHECK(optional2 != optional3);
        CHECK(optional1 < optional3);
        CHECK(optional3 > optional1);
        CHECK(optional2 < optional3);
        optional3 = 3.0;
        CHECK(optional2 == optional3);
    }
}