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
    retro::Polymorphic<Base> polymorphic1 = Derived1(42);
    CHECK(polymorphic1->get_value() == 42);
    CHECK(polymorphic1.get_size() == sizeof(Derived1));

    retro::Polymorphic<Base> polymorphic2(std::in_place_type<Derived2>, VALUE_ARRAY1);
    CHECK(polymorphic2->get_value() == 120);
    CHECK(polymorphic2.get_size() == sizeof(Derived2));

    polymorphic1 = polymorphic2;
    CHECK(polymorphic1->get_value() == 120);

    polymorphic2.emplace<Derived1>(40);
    polymorphic1 = polymorphic2;
    CHECK(polymorphic1->get_value() == 40);

    polymorphic1 = retro::Polymorphic<Base>(std::in_place_type<Derived2>, VALUE_ARRAY2);
    CHECK(polymorphic1->get_value() == 240);

    // Here we want to check that during reassignment the destructor for a different type gets invoked
    auto value = std::make_shared<int>(4);
    std::weak_ptr<int> weakValue = value;
    polymorphic1 = Derived3(std::move(value));
    CHECK(polymorphic1->get_value() == 4);

    polymorphic1 = retro::Polymorphic<Base>();
    CHECK(polymorphic1->get_value() == 0);
    CHECK(weakValue.expired());

    // Here we want to verify that the polymorphic values can copy into like types when using small storage
    polymorphic1 = Derived1(12);
    polymorphic2 = Derived1(64);
    CHECK(polymorphic1->get_value() == 12);
    CHECK(polymorphic2->get_value() == 64);

    polymorphic1 = polymorphic2;
    CHECK(polymorphic1->get_value() == 64);
    polymorphic1 = retro::Polymorphic<Base>(std::in_place_type<Derived1>, 100);
    CHECK(polymorphic1->get_value() == 100);

    // Here we're going to do the same thing with large storage
    polymorphic1.emplace<Derived2>(VALUE_ARRAY1);
    polymorphic2.emplace<Derived2>(VALUE_ARRAY2);
    CHECK(polymorphic1->get_value() == 120);
    CHECK(polymorphic2->get_value() == 240);

    polymorphic1 = polymorphic2;
    CHECK(polymorphic1->get_value() == 240);
    polymorphic1 = retro::Polymorphic<Base>(std::in_place_type<Derived2>, VALUE_ARRAY1);
    CHECK(polymorphic1->get_value() == 120);

    // Check that dereferencing works correctly
    auto &dereferenced1 = *polymorphic1;
    CHECK(dereferenced1.get_value() == 120);

    const retro::Polymorphic<Base> polymorphic3 = retro::Polymorphic<Base>(std::in_place_type<Derived1>, 150);
    auto &dereferenced2 = *polymorphic3;
    CHECK(dereferenced2.get_value() == 150);

    polymorphic1 = retro::Polymorphic<Base>(std::in_place_type<Derived2>, VALUE_ARRAY1);
    const retro::Polymorphic<Base> polymorphic4 = polymorphic1;
    auto &dereferenced3 = *polymorphic4;
    CHECK(dereferenced3.get_value() == 120);
}