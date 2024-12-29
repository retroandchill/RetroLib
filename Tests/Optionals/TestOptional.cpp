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
        Retro::Optional<int> optional1;
        CHECK_FALSE(optional1.HasValue());
        CHECK(optional1 == std::nullopt);
        CHECK(std::nullopt == optional1);
        CHECK(optional1 <= std::nullopt);
        CHECK(std::nullopt <= optional1);
        CHECK(optional1 >= std::nullopt);
        CHECK(std::nullopt >= optional1);
        CHECK_THROWS_AS(optional1.Value(), std::bad_optional_access);
        CHECK_THROWS_AS(std::as_const(optional1.Value()), std::bad_optional_access);
        CHECK_THROWS_AS(std::move(optional1).Value(), std::bad_optional_access);

        Retro::Optional optional2 = 3;
        CHECK(optional2.HasValue());
        CHECK(optional2 != std::nullopt);
        CHECK(std::nullopt != optional2);
        CHECK(optional2 > std::nullopt);
        CHECK(std::nullopt < optional2);
        CHECK(optional2 == 3);
        CHECK(optional2.Value() == 3);
        CHECK(*optional2 == 3);

        CHECK(optional1.ValueOr(4) == 4);
        CHECK(std::as_const(optional1).ValueOr(4) == 4);
        CHECK(optional2.ValueOr(4) == 3);
        CHECK(std::as_const(optional2).ValueOr(4) == 3);

        Retro::Optional<std::string> optional3 = "Hello";
        CHECK(optional3.HasValue());
        CHECK(optional3 == "Hello");
        CHECK(std::as_const(optional3).Value() == "Hello");
        CHECK(std::move(optional3).Value() == "Hello");
        CHECK(*std::as_const(optional3) == "Hello");
        CHECK(*std::move(optional3) == "Hello");

        optional3.Reset();
        CHECK_FALSE(optional3.HasValue());

        optional3.Emplace("New string");
        CHECK(optional3.HasValue());
        CHECK(optional3 != "Hello");
        CHECK(optional3 == "New string");
        CHECK(optional3->length() == 10);
        CHECK(std::as_const(optional3)->length() == 10);

        auto optional4 = std::move(optional3);
        REQUIRE(optional3.HasValue());
        CHECK(optional3->empty());

        auto optional5 = optional4;
        REQUIRE(optional4.HasValue());
        CHECK(optional4 == "New string");

        optional3 = std::nullopt;
        CHECK_FALSE(optional3.HasValue());

        auto optional6 = optional3;
        CHECK_FALSE(optional6.HasValue());
        auto optional7 = std::move(optional3);
        CHECK_FALSE(optional3.HasValue());

        optional4 = optional3;
        CHECK_FALSE(optional3.HasValue());
        optional5 = std::move(optional3);
        CHECK_FALSE(optional3.HasValue());

        optional3.Emplace("New value");
        optional6 = optional3;
        CHECK(optional6.HasValue());
        optional5 = "Test string";
        optional5 = std::move(optional3);
        CHECK(optional5.HasValue());
    }

    SECTION("Can compare optionals between each other") {
        Retro::Optional<int> optional1 = std::nullopt;
        Retro::Optional optional2 = 3;
        static_assert(Retro::EqualityComparable<int, int>);
        static_assert(Retro::EqualityComparable<decltype(optional1)::ValueType, decltype(optional2)::ValueType>);
        CHECK_FALSE(optional1 == optional2);
        CHECK(optional1 < optional2);
        CHECK(optional2 > optional1);

        Retro::Optional optional3 = 7.0;
        CHECK(optional1 != optional3);
        CHECK(optional2 != optional3);
        CHECK(optional1 < optional3);
        CHECK(optional3 > optional1);
        CHECK(optional2 < optional3);
        optional3 = 3.0;
        CHECK(optional2 == optional3);
    }

    SECTION("Swap operation works") {
        Retro::Optional optional1 = 3;
        Retro::Optional optional2 = 7;
        swap(optional1, optional2);
        CHECK(optional1 == 7);
        CHECK(optional2 == 3);

        Retro::Optional<std::string> optional3 = "Hello";
        Retro::Optional<std::string> optional4 = "World";
        swap(optional3, optional4);
        CHECK(optional3 == "World");
        CHECK(optional4 == "Hello");

        optional3.Reset();
        swap(optional3, optional4);
        CHECK_FALSE(optional4.HasValue());
        CHECK(optional3 == "Hello");

        optional3.Reset();
        swap(optional3, optional4);
        CHECK_FALSE(optional3.HasValue());
        CHECK_FALSE(optional4.HasValue());

        optional3 = "Hello world";
        std::string value = *std::move(optional3);
        CHECK(value == "Hello world");
    }

    SECTION("Can have an optional of a reference") {
        Retro::Optional Optional1 = 3;
        auto Optional2 = Retro::Optionals::MakeOptionalReference(Optional1);
        CHECK(Optional2.HasValue());
        CHECK(*Optional2 == 3);
        CHECK(std::addressof(std::as_const(*Optional2)) == std::addressof(*Optional1));

        Optional2.Reset();
        CHECK_FALSE(Optional2.HasValue());

        int FreeVariable = 7;
        Optional2 = FreeVariable;
        CHECK(Optional2.HasValue());
        CHECK(*Optional2 == 7);

        FreeVariable++;
        CHECK(*Optional2 == 8);

        int OtherVariable = 4;
        Retro::Optional<int &> Optional3 = OtherVariable;
        CHECK(Optional3.HasValue());
        swap(Optional2, Optional3);
        CHECK(*Optional3 == 8);
        CHECK(FreeVariable == 4);
        CHECK(OtherVariable == 8);

        Optional3.Reset();
        swap(Optional3, Optional2);
        CHECK(*Optional3 == 4);
        CHECK_FALSE(Optional2.HasValue());
        CHECK(FreeVariable == 4);

        std::string TestString = "Hello world";
        Retro::Optional<std::string &> Optional4 = TestString;
        CHECK(Optional4.HasValue());
        CHECK(Optional4->length() == 11);
        CHECK(std::as_const(Optional4)->length() == 11);
    }
}