/**
 * @file PolymorphicTest.cpp
 * @brief Test for the Polymorphic class
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#include <catch2/catch_test_macros.hpp>

#if RETROLIB_WITH_MODULES
import std;
import RetroLib;
#else
#include "RetroLib/Utils/Polymorphic.h"

#include <array>
#include <memory>
#endif

class Base {
  public:
    virtual ~Base() = default;

    virtual int get_value() const {
        return 0;
    }
};

class Derived1 : public Base {
  public:
    explicit Derived1(int value) : value(value) {
    }

    int get_value() const override {
        return value;
    }

  private:
    int value;
};

class Derived2 : public Base {
  public:
    explicit Derived2(const std::array<int, 15> &values) : values(values) {
    }

    int get_value() const override {
        int value = 0;
        for (int val : values) {
            value += val;
        }
        return value;
    }

  private:
    std::array<int, 15> values;
};

class Derived3 : public Base {
  public:
    explicit Derived3(std::shared_ptr<int> value) : value(std::move(value)) {
    }

    int get_value() const override {
        return *value;
    }

  private:
    std::shared_ptr<int> value;
};

constexpr std::array VALUE_ARRAY1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
constexpr std::array VALUE_ARRAY2 = {2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30};

TEST_CASE("Polymorphic types can be instantiated and copied", "[utils]") {
    // We want to test that we can assign different polymorphic values into each other
    Retro::Polymorphic<Base> polymorphic1 = Derived1(42);
    CHECK(polymorphic1->get_value() == 42);
    CHECK(polymorphic1.GetSize() == sizeof(Derived1));

    Retro::Polymorphic<Base> polymorphic2(std::in_place_type<Derived2>, VALUE_ARRAY1);
    CHECK(polymorphic2->get_value() == 120);
    CHECK(polymorphic2.GetSize() == sizeof(Derived2));

    polymorphic1 = polymorphic2;
    CHECK(polymorphic1->get_value() == 120);

    polymorphic2.Emplace<Derived1>(40);
    polymorphic1 = polymorphic2;
    CHECK(polymorphic1->get_value() == 40);

    polymorphic1 = Retro::Polymorphic<Base>(std::in_place_type<Derived2>, VALUE_ARRAY2);
    CHECK(polymorphic1->get_value() == 240);

    // Here we want to check that during reassignment the destructor for a different type gets invoked
    auto value = std::make_shared<int>(4);
    std::weak_ptr<int> weakValue = value;
    polymorphic1 = Derived3(std::move(value));
    CHECK(polymorphic1->get_value() == 4);

    polymorphic1 = Retro::Polymorphic<Base>();
    CHECK(polymorphic1->get_value() == 0);
    CHECK(weakValue.expired());

    // Here we want to verify that the polymorphic values can copy into like types when using small storage
    polymorphic1 = Derived1(12);
    polymorphic2 = Derived1(64);
    CHECK(polymorphic1->get_value() == 12);
    CHECK(polymorphic2->get_value() == 64);

    polymorphic1 = polymorphic2;
    CHECK(polymorphic1->get_value() == 64);
    polymorphic1 = Retro::Polymorphic<Base>(std::in_place_type<Derived1>, 100);
    CHECK(polymorphic1->get_value() == 100);

    // Here we're going to do the same thing with large storage
    polymorphic1.Emplace<Derived2>(VALUE_ARRAY1);
    polymorphic2.Emplace<Derived2>(VALUE_ARRAY2);
    CHECK(polymorphic1->get_value() == 120);
    CHECK(polymorphic2->get_value() == 240);

    polymorphic1 = polymorphic2;
    CHECK(polymorphic1->get_value() == 240);
    polymorphic1 = Retro::Polymorphic<Base>(std::in_place_type<Derived2>, VALUE_ARRAY1);
    CHECK(polymorphic1->get_value() == 120);

    // Check that dereferencing works correctly
    auto &dereferenced1 = *polymorphic1;
    CHECK(dereferenced1.get_value() == 120);

    const Retro::Polymorphic<Base> polymorphic3 = Retro::Polymorphic<Base>(std::in_place_type<Derived1>, 150);
    auto &dereferenced2 = *polymorphic3;
    CHECK(dereferenced2.get_value() == 150);

    polymorphic1 = Retro::Polymorphic<Base>(std::in_place_type<Derived2>, VALUE_ARRAY1);
    const Retro::Polymorphic<Base> polymorphic4 = polymorphic1;
    auto &dereferenced3 = *polymorphic4;
    CHECK(dereferenced3.get_value() == 120);
}

TEST_CASE("Polymorphic has an intrusive unset optional state", "[utils]") {
    static_assert(Retro::HasIntrusiveUnsetState<Retro::Polymorphic<Base>>);
    static_assert(sizeof(Retro::Polymorphic<Base>) == sizeof(Retro::Optional<Retro::Polymorphic<Base>>));
    Retro::Optional<Retro::Polymorphic<Base>> Optional1;
    CHECK_FALSE(Optional1.HasValue());
    Optional1.Emplace(std::in_place_type<Derived1>, 12);
    REQUIRE(Optional1.HasValue());
    auto ObtainedValue1 = Optional1->Get();
    CHECK(dynamic_cast<Derived1 *>(ObtainedValue1) != nullptr);
    auto ObtainedValue2 = std::as_const(Optional1)->Get();
    CHECK(dynamic_cast<const Derived1 *>(ObtainedValue2) != nullptr);

    auto ObtainedValue3 = *Optional1;
    CHECK(ObtainedValue3->get_value() == 12);
    auto ObtainedValue4 = *std::as_const(Optional1);
    CHECK(ObtainedValue4->get_value() == 12);

    auto ObtainedValue5 = *Retro::Optional(Retro::Polymorphic<Base>(std::in_place_type<Derived1>, 24));
    CHECK(ObtainedValue5->get_value() == 24);

    auto Optional2 = Optional1;
    Optional1.Reset();
    CHECK(Optional2.HasValue());
    CHECK_FALSE(Optional1.HasValue());

    swap(Optional1, Optional2);
    CHECK_FALSE(Optional2.HasValue());
    CHECK(Optional1.HasValue());
}