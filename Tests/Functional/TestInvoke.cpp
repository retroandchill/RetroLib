/**
 * @file TestInvoke.cpp
 * @brief Tests the constexpr invoke functions.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#include <catch2/catch_test_macros.hpp>

#if RETROLIB_WITH_MODULES
import std;
import RetroLib;
#else
#include "RetroLib/Functional/Invoke.h"
#endif

static int add_numbers(int a, int b, int c) {
    return a + b + c;
}

struct InvokeTestStruct {
    int data_member;

    int get_data_member() const {
        return data_member;
    }
};

TEST_CASE("Test the constexpr invoke works in all cases") {
    SECTION("Verify that the static invoke works in this case") {
        CHECK(retro::invoke<add_numbers>(1, 2, 3) == 6);
    }

    SECTION("Verify it can be used on class members") {
        InvokeTestStruct object = { 5 };
        CHECK(retro::invoke<&InvokeTestStruct::get_data_member>(object) == 5);
        CHECK(retro::invoke<&InvokeTestStruct::data_member>(object) == 5);
    }

    SECTION("Verify it can be used on class members with a reference wrapper") {
        InvokeTestStruct object = { 5 };
        std::reference_wrapper wrapped = object;
        CHECK(retro::invoke<&InvokeTestStruct::get_data_member>(wrapped) == 5);
        CHECK(retro::invoke<&InvokeTestStruct::data_member>(wrapped) == 5);
    }

    SECTION("Verify it can be used on class members with a raw pointer") {
        InvokeTestStruct object = { 5 };
        CHECK(retro::invoke<&InvokeTestStruct::get_data_member>(&object) == 5);
        CHECK(retro::invoke<&InvokeTestStruct::data_member>(&object) == 5);
    }

    SECTION("Verify it can be used on class members with a smart pointer") {
        auto object = std::make_unique<InvokeTestStruct>(5);
        CHECK(retro::invoke<&InvokeTestStruct::get_data_member>(object) == 5);
        CHECK(retro::invoke<&InvokeTestStruct::data_member>(object) == 5);
    }
}