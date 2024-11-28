/**
 * @file PolymorphicTest.cpp
 * @brief Test for the Polymorphic class
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#include <catch2/catch_test_macros.hpp>


#ifdef RETROLIB_WITH_MODULES
import std;
import RetroLib;
#else
#include <array>
#include <memory>
#include "RetroLib/Utils/Polymorphic.h"
#endif

class Base {
public:
    virtual ~Base() = default;

    virtual int getValue() const {
        return 0;
    }
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
    explicit Derived2(const std::array<int, 15>& values) : values(values) {}

    int getValue() const override {
        int value = 0;
        for (int val: values) {
            value += val;
        }
        return value;
    }

private:
    std::array<int, 15> values;
};

class Derived3 : public Base {
public:
    explicit Derived3(std::shared_ptr<int> value) : value(std::move(value)) {}

    int getValue() const override {
        return *value;
    }

private:
    std::shared_ptr<int> value;
};

TEST_CASE("Polymorphic types can be instantiated and copied", "[utils]") {
    Retro::Polymorphic<Base> polymorphic1 = Derived1(42);
    CHECK(polymorphic1->getValue() == 42);

    Retro::Polymorphic<Base> polymorphic2(std::in_place_type<Derived2>,
            std::array{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
    CHECK(polymorphic2->getValue() == 120);

    polymorphic1 = polymorphic2;
    CHECK(polymorphic1->getValue() == 120);

    auto value = std::make_shared<int>(4);
    std::weak_ptr<int> weakValue = value;
    polymorphic1 = Derived3(std::move(value));
    CHECK(polymorphic1->getValue() == 4);

    polymorphic1 = Retro::Polymorphic<Base>();
    CHECK(polymorphic1->getValue() == 0);
    CHECK(weakValue.expired());
}