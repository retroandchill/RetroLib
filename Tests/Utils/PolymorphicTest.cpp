/**
 * @file PolymorphicTest.cpp
 * @brief Test for the Polymorphic class
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#include <catch2/catch_test_macros.hpp>

import std;
import RetroLib;

class Base {
public:
    virtual ~Base() = default;

    virtual int getValue() const = 0;
};

class Derived1 : public Base {
public:
    explicit Derived1(int value) : value(value) {}

    int getValue() const override {
        return value;
    }

private:
    int value;
};

class Derived2 : public Base {
public:
    explicit Derived2(const std::array<int, 10>& values) : values(values) {}

    int getValue() const override {
        int value = 0;
        for (int val: values) {
            value += val;
        }
        return value;
    }

private:
    std::array<int, 10> values;
};

TEST_CASE("Polymorphic types can be instantiated and copied", "[utils]") {
    Retro::Polymorphic<Base> polymorphic1 = Derived1(42);
    CHECK(polymorphic1->getValue() == 42);

    Retro::Polymorphic<Base> polymorphic2(std::in_place_type<Derived2>, std::array{1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    CHECK(polymorphic2->getValue() == 55);

    polymorphic1 = polymorphic2;
    CHECK(polymorphic1->getValue() == 55);
}