/**
 * @file TestInstanceOf.cpp
 * @brief Get for the instance_of functor.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#include <catch2/catch_test_macros.hpp>

#if RETROLIB_WITH_MODULES
import std;
import RetroLib;
#else
#include "RetroLib/Casting/ByteCast.h"
#include "RetroLib/Casting/DynamicCast.h"
#include "RetroLib/Casting/StaticCast.h"
#include "RetroLib/Casting/InstanceOf.h"

#include <memory>
#endif

namespace instanceof_test {
    struct Base {
        virtual ~Base() = default;
        virtual int foo() = 0;
    };

    struct Derived1 : Base {
        int foo() override {
            return value1;
        }

        int value1 = 3;
    };
    struct Derived2 : Base {
        int foo() override {
            return value2;
        }

        int value2 = 4;
    };
} // namespace instanceof_test

using namespace instanceof_test;

TEST_CASE("Check to see if we can convert between types statically", "[utils]") {
    SECTION("Can convert between numberic types") {
        CHECK(Retro::StaticCast<double>(4) == 4.0);
        CHECK(Retro::StaticCast<int>(4.0) == 4);
    }

    SECTION("Can perform implicit conversions that call constructors") {
        CHECK(Retro::StaticCast<std::string>("Hello world") == "Hello world");
        CHECK(Retro::StaticCast<Retro::Optional<int>>(4) == 4);
    }

    SECTION("Can convert to pointers") {
        Derived1 derived1;
        auto base = Retro::StaticCast<Base *>(&derived1);
        CHECK(base == &derived1);
        CHECK(Retro::StaticCast<Derived1 *>(base) == &derived1);
        auto opaque = Retro::StaticCast<void *>(base);
        CHECK(opaque == &derived1);
        CHECK(Retro::StaticCast<Derived1 *>(opaque) == &derived1);
        CHECK(std::addressof(Retro::ByteCast<Derived1>(opaque)) == &derived1);
    }
}

TEST_CASE("Can check if a polymorphic type is an instance of another type", "[utils]") {
    SECTION("Can work with raw pointers") {
        Derived1 value1;
        Derived2 value2;
        Base *ptr = nullptr;
        Base *valid_ptr1 = &value1;
        Base *valid_ptr2 = &value2;

        CHECK_FALSE(Retro::InstanceOf<Base>(ptr));
        CHECK_FALSE(Retro::InstanceOf<Derived1>(ptr));
        CHECK_FALSE(Retro::InstanceOf<Derived2>(ptr));

        CHECK(Retro::InstanceOf<Base>(valid_ptr1));
        CHECK(Retro::InstanceOf<Derived1>(valid_ptr1));
        CHECK_FALSE(Retro::InstanceOf<Derived2>(valid_ptr1));

        CHECK(Retro::InstanceOf<Base>(valid_ptr2));
        CHECK_FALSE(Retro::InstanceOf<Derived1>(valid_ptr2));
        CHECK(Retro::InstanceOf<Derived2>(valid_ptr2));
    }

    SECTION("Handles nullptr literals") {
        CHECK_FALSE(Retro::InstanceOf<Derived1>(nullptr));
        CHECK_FALSE(Retro::InstanceOf<Derived2>(nullptr));
    }

    SECTION("Can work with wrapped pointer types (smart pointers)") {
        std::unique_ptr<Base> ptr = nullptr;
        std::unique_ptr<Base> valid_ptr1 = std::make_unique<Derived1>();
        std::unique_ptr<Base> valid_ptr2 = std::make_unique<Derived2>();

        CHECK_FALSE(Retro::InstanceOf<Base>(ptr));
        CHECK_FALSE(Retro::InstanceOf<Derived1>(ptr));
        CHECK_FALSE(Retro::InstanceOf<Derived2>(ptr));

        CHECK(Retro::InstanceOf<Base>(valid_ptr1));
        CHECK(Retro::InstanceOf<Derived1>(valid_ptr1));
        CHECK_FALSE(Retro::InstanceOf<Derived2>(valid_ptr1));

        CHECK(Retro::InstanceOf<Base>(valid_ptr2));
        CHECK_FALSE(Retro::InstanceOf<Derived1>(valid_ptr2));
        CHECK(Retro::InstanceOf<Derived2>(valid_ptr2));
    }

    SECTION("Can work with a polymorphic value instance") {
        Retro::Polymorphic<Base> valid_ptr1(std::in_place_type<Derived1>);
        Retro::Polymorphic<Base> valid_ptr2(std::in_place_type<Derived2>);

        CHECK(Retro::InstanceOf<Base>(valid_ptr1));
        CHECK(Retro::InstanceOf<Derived1>(valid_ptr1));
        CHECK_FALSE(Retro::InstanceOf<Derived2>(valid_ptr1));

        CHECK(Retro::InstanceOf<Base>(valid_ptr2));
        CHECK_FALSE(Retro::InstanceOf<Derived1>(valid_ptr2));
        CHECK(Retro::InstanceOf<Derived2>(valid_ptr2));
    }
}

TEST_CASE("Can check if a we can cast between types", "[utils]") {
    SECTION("Can work with raw pointers") {
        Derived1 value1;
        Derived2 value2;
        Base *ptr = nullptr;
        Base *valid_ptr1 = &value1;
        Base *valid_ptr2 = &value2;

        CHECK_FALSE(Retro::DynamicCast<Base>(ptr).HasValue());
        CHECK_FALSE(Retro::DynamicCast<Derived1>(ptr).HasValue());
        CHECK_FALSE(Retro::DynamicCast<Derived2>(ptr).HasValue());

        CHECK(Retro::DynamicCast<Base>(valid_ptr1).HasValue());
        CHECK(Retro::DynamicCast<Derived1>(valid_ptr1).HasValue());
        CHECK_FALSE(Retro::DynamicCast<Derived2>(valid_ptr1).HasValue());

        CHECK(Retro::DynamicCast<Base>(valid_ptr2).HasValue());
        CHECK_FALSE(Retro::DynamicCast<Derived1>(valid_ptr2).HasValue());
        CHECK(Retro::DynamicCast<Derived2>(valid_ptr2).HasValue());
    }

    SECTION("Handles nullptr literals") {
        CHECK_FALSE(Retro::DynamicCast<Derived1>(nullptr).HasValue());
        CHECK_FALSE(Retro::DynamicCast<Derived2>(nullptr).HasValue());
    }

    SECTION("Can work with wrapped pointer types (smart pointers)") {
        std::unique_ptr<Base> ptr = nullptr;
        std::unique_ptr<Base> valid_ptr1 = std::make_unique<Derived1>();
        std::unique_ptr<Base> valid_ptr2 = std::make_unique<Derived2>();

        CHECK_FALSE(Retro::DynamicCast<Base>(ptr).HasValue());
        CHECK_FALSE(Retro::DynamicCast<Derived1>(ptr).HasValue());
        CHECK_FALSE(Retro::DynamicCast<Derived2>(ptr).HasValue());

        CHECK(Retro::DynamicCast<Base>(valid_ptr1).HasValue());
        CHECK(Retro::DynamicCast<Derived1>(valid_ptr1).HasValue());
        CHECK_FALSE(Retro::DynamicCast<Derived2>(valid_ptr1).HasValue());

        CHECK(Retro::DynamicCast<Base>(valid_ptr2).HasValue());
        CHECK_FALSE(Retro::DynamicCast<Derived1>(valid_ptr2).HasValue());
        CHECK(Retro::DynamicCast<Derived2>(valid_ptr2).HasValue());
    }

    SECTION("Can work with a polymorphic value instance") {
        Retro::Polymorphic<Base> valid_ptr1(std::in_place_type<Derived1>);
        Retro::Polymorphic<Base> valid_ptr2(std::in_place_type<Derived2>);

        CHECK(Retro::InstanceOf<Base>(valid_ptr1));
        CHECK(Retro::InstanceOf<Derived1>(valid_ptr1));
        CHECK_FALSE(Retro::InstanceOf<Derived2>(valid_ptr1));

        CHECK(Retro::InstanceOf<Base>(valid_ptr2));
        CHECK_FALSE(Retro::InstanceOf<Derived1>(valid_ptr2));
        CHECK(Retro::InstanceOf<Derived2>(valid_ptr2));
    }
}