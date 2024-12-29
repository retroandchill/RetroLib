/**
 * @file TestFilterOptional.cpp
 * @brief Test for the optional filter operation.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#include <catch2/catch_test_macros.hpp>

#if RETROLIB_WITH_MODULES
import RetroLib;
import std;
#else
#include "RetroLib/Optionals/AndThen.h"
#include "RetroLib/Optionals/Filter.h"
#include "RetroLib/Optionals/IfPresent.h"
#include "RetroLib/Optionals/IfPresentOrElse.h"
#include "RetroLib/Optionals/IsSet.h"
#include "RetroLib/Optionals/OrElse.h"
#include "RetroLib/Optionals/OrElseGet.h"
#include "RetroLib/Optionals/OrElseThrow.h"
#include "RetroLib/Optionals/OrElseValue.h"
#include "RetroLib/Optionals/PtrOrNull.h"
#include "RetroLib/Optionals/To.h"
#include "RetroLib/Optionals/Transform.h"
#include "RetroLib/Optionals/Value.h"

#include <array>
#endif

TEST_CASE("Can filter an optional value", "[optionals]") {
    SECTION("Can filter on a constant functor") {
        constexpr auto is_even = [](int x, int y) { return x % y == 0; };
        std::optional odd = 3;
        std::optional even = 4;
        std::optional<int> empty;
        auto filtered_odd = odd | Retro::Optionals::Filter<is_even>(2);
        CHECK_FALSE(filtered_odd.has_value());
        auto filtered_even = even | Retro::Optionals::Filter<is_even>(2);
        CHECK(filtered_even.has_value());
        auto filtered_empty = empty | Retro::Optionals::Filter<is_even>(2);
        CHECK_FALSE(filtered_empty.has_value());

        auto filtered_rvalue_odd = std::optional(5) | Retro::Optionals::Filter<is_even>(2);
        CHECK_FALSE(filtered_rvalue_odd.has_value());
        auto filtered_rvalue_even = std::optional(6) | Retro::Optionals::Filter<is_even>(2);
        CHECK(filtered_rvalue_even.has_value());
        auto filtered_empty_rvalue = std::optional<int>() | Retro::Optionals::Filter<is_even>(2);
        CHECK_FALSE(filtered_empty_rvalue.has_value());
    }

    SECTION("Can filter on a runtime functor") {
        auto greater_than = [](int x, int y) { return x > y; };
        std::optional odd = 3;
        std::optional even = 4;
        std::optional<int> empty;
        static_assert(Retro::Optionals::OptionalType<decltype(odd)>);
        auto filtered_odd = odd | Retro::Optionals::Filter(greater_than, 4);
        CHECK_FALSE(filtered_odd.has_value());
        auto filtered_even = even | Retro::Optionals::Filter(greater_than, 3);
        CHECK(filtered_even.has_value());
        auto filtered_empty = empty | Retro::Optionals::Filter(greater_than, 3);
        CHECK_FALSE(filtered_empty.has_value());

        auto filtered_rvalue_odd = std::optional(5) | Retro::Optionals::Filter(greater_than, 7);
        CHECK_FALSE(filtered_rvalue_odd.has_value());
        auto filtered_rvalue_even = std::optional(6) | Retro::Optionals::Filter(greater_than, 4);
        CHECK(filtered_rvalue_even.has_value());
        auto filtered_rvalue_empty = std::optional<int>() | Retro::Optionals::Filter(greater_than, 4);
        CHECK_FALSE(filtered_rvalue_empty.has_value());
    }
}

TEST_CASE("Can transform an optional between various types", "[optionals]") {
    constexpr auto multiply_value = [](int x, int y) { return x * y; };
    SECTION("Can transform using a constant functor") {
        std::optional value = 3;
        auto transformed = value | Retro::Optionals::Transform<multiply_value>(2);
        CHECK(transformed.has_value());
        CHECK(transformed.value() == 6);

        std::optional<int> empty_opt;
        auto empty_transformed = empty_opt | Retro::Optionals::Transform<multiply_value>(2);
        CHECK_FALSE(empty_transformed.has_value());
    }

    SECTION("Can transform using a runtime defined functor") {
        std::optional value = 6;
        auto transformed = value | Retro::Optionals::Transform(multiply_value, 5);
        CHECK(transformed.has_value());
        CHECK(transformed.value() == 30);
    }

    SECTION("Can transform a value returning a reference to hold said reference") {
        std::array values = {1, 2, 3, 4, 5};
        std::optional index = 4;
        auto transformed =
            index | Retro::Optionals::Transform([&values](int i) -> decltype(auto) { return values[i]; });
        CHECK(transformed.has_value());
        CHECK(transformed.value() == 5);
    }

    SECTION("Can transform a value returning a pointer to hold a reference to that pointer") {
        std::array values = {1, 2, 3, 4, 5};
        auto transformer = [&values](int i) {
            if (values.size() > i) {
                return &values[i];
            }

            return static_cast<int *>(nullptr);
        };
        Retro::Optional index = 2;
        auto transformed = index | Retro::Optionals::Transform(transformer);
        CHECK(transformed.HasValue());
        CHECK(transformed.Value() == 3);

        index = 6;
        auto second_pass = index | Retro::Optionals::Transform(transformer);
        CHECK_FALSE(second_pass.HasValue());
    }
}

TEST_CASE("Can flat map down an optional result", "[optionals]") {
    SECTION("Can return an optional of the same type") {
        constexpr auto mapper = [](int x) {
            if (x > 0) {
                return std::optional(x * 2);
            }

            return std::optional<int>();
        };

        auto mapped1 = std::optional(4) | Retro::Optionals::AndThen(mapper);
        CHECK(mapped1.has_value());
        CHECK(mapped1.value() == 8);

        auto mapped2 = std::optional(-3) | Retro::Optionals::AndThen(mapper);
        CHECK_FALSE(mapped2.has_value());

        auto mapped3 = std::optional<int>() | Retro::Optionals::AndThen(mapper);
        CHECK_FALSE(mapped3.has_value());
    }

    SECTION("Can return an optional of a different type") {
        constexpr auto mapper = [](int x) {
            if (x > 0) {
                return Retro::Optional(x * 2);
            }

            return Retro::Optional<int>();
        };

        auto mapped1 = std::optional(4) | Retro::Optionals::AndThen<mapper>();
        CHECK(mapped1.HasValue());
        CHECK(mapped1.Value() == 8);

        auto mapped2 = std::optional(-3) | Retro::Optionals::AndThen<mapper>();
        CHECK_FALSE(mapped2.HasValue());

        auto mapped3 = std::optional<int>() | Retro::Optionals::AndThen<mapper>();
        CHECK_FALSE(mapped3.HasValue());
    }
}

TEST_CASE("Test getting a value or throwing an exception if its empty", "[optionals]") {
    std::optional value1 = 4;
    CHECK_NOTHROW(value1 | Retro::Optionals::OrElseThrow());

    std::optional<int> value2 = std::nullopt;
    CHECK_THROWS_AS(value2 | Retro::Optionals::OrElseThrow(), std::bad_optional_access);

    constexpr auto alt_throw = [](std::string_view message) { return std::runtime_error(message.data()); };
    CHECK_THROWS_AS(value2 | Retro::Optionals::OrElseThrow<alt_throw>("Could not get value!"), std::runtime_error);
    CHECK_THROWS_AS(value2 | Retro::Optionals::OrElseThrow(alt_throw, "Could not get value!"), std::runtime_error);
}

TEST_CASE("Test getting a value without any runtime checks (very unsafe)", "[optionals]") {
    std::optional value1 = 4;
    auto result = value1 | Retro::Optionals::Value;
    CHECK(result == 4);
}

TEST_CASE("Test getting a value or a null pointer", "[optionals]") {
    std::optional value1 = 4;
    auto result1 = value1 | Retro::Optionals::PtrOrNull;
    REQUIRE(result1 != nullptr);
    CHECK(*result1 == 4);

    std::optional<int> value2 = std::nullopt;
    auto result2 = value2 | Retro::Optionals::PtrOrNull;
    CHECK(result2 == nullptr);

    auto result3 = Retro::Optionals::MakeOptionalReference(value2) | Retro::Optionals::PtrOrNull;
    CHECK(result3 == nullptr);

    auto result4 = Retro::Optionals::OfNullable(result1) | Retro::Optionals::PtrOrNull;
    CHECK(result4 == result1);
}

TEST_CASE("Can check if an optional value is set", "[optionals]") {
    std::optional value1 = 4;
    CHECK(value1 | Retro::Optionals::IsSet);

    std::optional<int> value2 = std::nullopt;
    CHECK_FALSE(value2 | Retro::Optionals::IsSet);
}

TEST_CASE("Can convert between various optional types", "[optionals]") {
    SECTION("Can convert between two optionals holding the same parameter") {
        std::optional value1 = 34;
        auto value2 = value1 | Retro::Optionals::To<Retro::Optional>();
        CHECK(value2.HasValue());
        CHECK(value2.Value() == 34);

        std::optional<int> value3 = std::nullopt;
        auto value4 = value3 | Retro::Optionals::To<Retro::Optional>();
        CHECK_FALSE(value4.HasValue());
    }

    SECTION("Can convert between two unlike optional types") {
        std::optional value1 = 34;
        auto value2 = value1 | Retro::Optionals::To<Retro::Optional<double>>();
        CHECK(value2.HasValue());
        CHECK(value2.Value() == 34.0);

        std::optional<int> value3 = std::nullopt;
        auto value4 = value3 | Retro::Optionals::To<Retro::Optional<double>>();
        CHECK_FALSE(value4.HasValue());
    }

    SECTION("Can convert from a reference-wrapped optional to a raw reference optional") {
        int ref_value = 34;
        std::optional value1 = std::ref(ref_value);
        auto value2 = value1 | Retro::Optionals::To<Retro::Optional>();
        CHECK(value2.HasValue());
        CHECK(value2.Value() == 34);

        std::optional<std::reference_wrapper<int>> value3 = std::nullopt;
        auto value4 = value3 | Retro::Optionals::To<Retro::Optional>();
        CHECK_FALSE(value4.HasValue());
    }

    SECTION("Can convert from a raw reference optional to a reference-wrapped optional") {
        int ref_value = 34;
        Retro::Optional<int &> value1 = ref_value;
        auto value2 = value1 | Retro::Optionals::To<std::optional>();
        CHECK(value2.has_value());
        CHECK(value2.value() == 34);

        std::optional<std::reference_wrapper<int>> value3 = std::nullopt;
        auto value4 = value3 | Retro::Optionals::To<std::optional>();
        CHECK_FALSE(value4.has_value());
    }
}

TEST_CASE("Can get the value inside of an optional or an alternative", "[optionals]") {
    SECTION("Can get the value of a value type out") {
        std::optional value1 = 34;
        auto value2 = value1 | Retro::Optionals::OrElseGet([] { return 5; });
        CHECK(value2 == 34);

        std::optional<int> value3 = std::nullopt;
        auto value4 = value3 | Retro::Optionals::OrElseGet([] { return 5; });
        CHECK(value4 == 5);
    }

    SECTION("Can get the value of a reference type out") {
        int ref_value = 34;
        int alt_value = 45;
        Retro::Optional<int &> value1 = ref_value;
        decltype(auto) value2 = value1 | Retro::Optionals::OrElseGet([&alt_value]() -> int & { return alt_value; });
        CHECK(value2 == 34);

        Retro::Optional<int &> value3 = std::nullopt;
        decltype(auto) value4 = value3 | Retro::Optionals::OrElseGet([&alt_value]() -> int & { return alt_value; });
        CHECK(value4 == 45);
    }

    SECTION("Can collapse two different types") {
        int ref_value = 34;
        Retro::Optional<int &> value1 = ref_value;
        decltype(auto) value2 = value1 | Retro::Optionals::OrElseGet([] { return 50.0; });
        CHECK(value2 == 34);

        Retro::Optional<int &> value3 = std::nullopt;
        decltype(auto) value4 = value3 | Retro::Optionals::OrElseGet([] { return 50.0; });
        CHECK(value4 == 50.0);
    }

    SECTION("Can get an optional out of the call") {
        std::optional value1 = 34;
        auto value2 = value1 | Retro::Optionals::OrElse([] { return std::optional(5); });
        REQUIRE(value2.has_value());
        CHECK(*value2 == 34);

        std::optional<int> value3 = std::nullopt;
        auto value4 = value3 | Retro::Optionals::OrElse([] { return std::optional(5); });
        REQUIRE(value4.has_value());
        CHECK(*value4 == 5);
    }
}

TEST_CASE("Can get the value or an alternative", "[optionals]") {
    SECTION("Can get basic values out") {
        std::optional value1 = 34;
        auto value2 = value1 | Retro::Optionals::OrElseValue(5);
        CHECK(value2 == 34);

        std::optional<int> value3 = std::nullopt;
        auto value4 = value3 | Retro::Optionals::OrElseValue(5);
        CHECK(value4 == 5);
    }

    SECTION("Can get references out") {
        int ref_value = 34;
        int alt_value = 45;
        Retro::Optional<int &> value1 = ref_value;
        decltype(auto) value2 = value1 | Retro::Optionals::OrElseValue(std::ref(alt_value));
        CHECK(value2 == 34);

        Retro::Optional<int &> value3 = std::nullopt;
        decltype(auto) value4 = value3 | Retro::Optionals::OrElseValue(std::ref(alt_value));
        CHECK(value4 == alt_value);
    }
}

TEST_CASE("Can execute on a value if said value is present", "[optionals]") {
    SECTION("Can execute if a value is present of do nothing") {
        int sum = 0;
        std::optional value1 = 34;
        value1 | Retro::Optionals::IfPresent([&sum](int value) { sum += value; });
        std::optional<int> value2 = std::nullopt;
        value2 | Retro::Optionals::IfPresent([&sum](int value) { sum += value; });
        CHECK(sum == 34);
    }

    SECTION("Can execute if a value is present, otherwise doing an alternate action") {
        int sum = 0;
        std::optional value1 = 34;
        value1 | Retro::Optionals::IfPresentOrElse([&sum](int value) { sum += value; }, [&sum] { sum += 5; });
        std::optional<int> value2 = std::nullopt;
        value2 | Retro::Optionals::IfPresentOrElse([&sum](int value) { sum += value; }, [&sum] { sum += 5; });
        CHECK(sum == 39);
    }
}