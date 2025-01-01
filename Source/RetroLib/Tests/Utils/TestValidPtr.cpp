/**
 * @file TestValidPtr.cpp
 * @brief Test for validating pointer values.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#include "Tests/TestAdapter.h"

#if RETROLIB_WITH_MODULES
import std;
import RetroLib;
#else
#include "RetroLib/Utils/ValidPtr.h"

#include <memory>
#endif

TEST_CASE("Can check if a pointer is valid or invalid", "[utils]") {
    SECTION("Can work with raw pointers") {
        int Value = 3;
        int *Ptr = nullptr;
        int *ValidPtr = &Value;

        CHECK_FALSE(Retro::ValidPtr(Ptr));
        CHECK(Retro::ValidPtr(ValidPtr));

        CHECK(Retro::InvalidPtr(Ptr));
        CHECK_FALSE(Retro::InvalidPtr(ValidPtr));
    }

    SECTION("Handles nullptr literals") {
        CHECK_FALSE(Retro::ValidPtr(nullptr));
        CHECK(Retro::InvalidPtr(nullptr));
    }

    SECTION("Can work with wrapped pointer types (smart pointers)") {
        std::unique_ptr<int> Ptr = nullptr;
        auto ValidPtr = std::make_unique<int>(3);

        CHECK_FALSE(Retro::ValidPtr(Ptr));
        CHECK(Retro::ValidPtr(ValidPtr));

        CHECK(Retro::InvalidPtr(Ptr));
        CHECK_FALSE(Retro::InvalidPtr(ValidPtr));
    }
}