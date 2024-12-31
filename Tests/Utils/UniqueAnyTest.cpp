/**
 * @file UniqueAnyTest.cpp
 * @brief Verifies that UniqueAny works as expected.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#include <catch2/catch_test_macros.hpp>

#if RETROLIB_WITH_MODULES
import std;
import RetroLib;
#else
#include "RetroLib/Utils/UniqueAny.h"

#include <array>
#include <memory>
#endif

TEST_CASE("Can create a move-only unique any type", "[utils]") {
    SECTION("Can move a value from one location to another") {
        auto SharedPointer = std::make_shared<int>(3);
        std::weak_ptr WeakPointer = SharedPointer;
        Retro::UniqueAny Any1 = std::move(SharedPointer);

        Retro::UniqueAny Any2 = std::move(Any1);
        Any2.Reset();

        CHECK(WeakPointer.expired());
    }

    SECTION("Can hold any type and change type as needed") {
        Retro::UniqueAny Any;
        CHECK_FALSE(Any.HasValue());

        std::any Value3 = 5;
        Any = 4;
        CHECK(Any.HasValue());
        REQUIRE(Any.GetType() == typeid(int));
        CHECK(Any.Get<int>() == 4);
        CHECK(std::as_const(Any).Get<int>() == 4);
        CHECK_THROWS_AS(Any.Get<float>(), std::bad_any_cast);
        CHECK_THROWS_AS(std::as_const(Any).Get<float>(), std::bad_any_cast);

        Any.Emplace<std::string>("Hello world");

        auto StringValue = Any.TryGet<std::string>();
        CHECK(StringValue.HasValue());
        CHECK(StringValue.Value() == "Hello world");

        auto StringViewValue = Any.TryGet<std::string_view>();
        CHECK_FALSE(StringViewValue.HasValue());

        auto ConstStringValue = std::as_const(Any).TryGet<std::string>();
        CHECK(ConstStringValue.HasValue());
        CHECK(ConstStringValue.Value() == "Hello world");

        auto ConstStringViewValue = std::as_const(Any).TryGet<std::string_view>();
        CHECK_FALSE(ConstStringViewValue.HasValue());

        Any.Emplace<std::array<std::string, 10>>();
        CHECK(Any.HasValue());
    }

    SECTION("Moving by assignment invalidates as well") {
        Retro::UniqueAny Any1(std::in_place_type<std::array<int, 20>>);

        Retro::UniqueAny Any2;
        Any2 = std::move(Any1);
        CHECK(Any2.HasValue());
        CHECK_FALSE(Any1.HasValue());
        CHECK(Any1.GetType() == typeid(void));
    }
}