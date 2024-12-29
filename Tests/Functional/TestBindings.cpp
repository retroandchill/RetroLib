/**
 * @file TestBindings.cpp
 * @brief Test the `bind_back` method to ensure that it works.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#include <catch2/catch_test_macros.hpp>

#if RETROLIB_WITH_MODULES
import std;
import RetroLib;
#else
#include "RetroLib/Functional/BindBack.h"
#include "RetroLib/Functional/BindFront.h"
#include "RetroLib/Functional/BindMethod.h"
#include "RetroLib/Functional/CreateBinding.h"

#include <vector>
#endif

static int add(int a, int b) {
    return a + b;
}

static void add_to_shared_back(int a, std::shared_ptr<int> ptr) {
    *ptr += a;
}

struct FunctionalObject {

    std::vector<int> &operator()(std::vector<int> &vector, int a, int b) const {
        vector.emplace_back(a);
        vector.emplace_back(b);
        return vector;
    }
};

constexpr FunctionalObject functor;

static int add_many(int a, int b, int c, int d) {
    return a + b + c + d;
}

static int add_numbers(int a, int b, int c) {
    return a + b + c;
}

class TestClass {

  public:
    int method(int value1, int value2, int value3) const {
        return value1 + value2 + value3;
    }

    int member = 9;
};

TEST_CASE("Can bind back to a runtime defined functional type", "[functional]") {
    SECTION("Binding back to a single argument works") {
        auto binding = Retro::BindBack(add, 4);
        CHECK(binding(3) == 7);
        CHECK(std::as_const(binding)(5) == 9);
        auto number = std::make_shared<int>(5);
        auto weakNumber = std::weak_ptr<int>(number);
        Retro::BindBack(add_to_shared_back, std::move(number))(4);
        CHECK(weakNumber.expired());
    }

    SECTION("Binding two arguments works") {
        std::vector<int> elements;
        auto binding = Retro::BindBack(functor, 3, 4);
        CHECK(std::addressof(elements) == std::addressof(binding(elements)));
        CHECK(elements.size() == 2);
        std::as_const(binding)(elements);
        CHECK(elements.size() == 4);
        Retro::BindBack(functor, 5, 6)(elements);
        CHECK(elements.size() == 6);
    }

    SECTION("Binding back to more than two arguments works") {
        auto binding = Retro::BindBack(&add_many, 4, 5, 6);
        CHECK(binding(3) == 18);
        CHECK(std::as_const(binding)(5) == 20);
        CHECK(Retro::BindBack(&add_many, 10, 20, 30)(5) == 65);
    }
}

TEST_CASE("Can bind back to a constexpr defined functional type", "[functional]") {
    SECTION("Binding back to a single argument works") {
        auto Binding = Retro::BindBack<add>(4);
        CHECK(Binding(3) == 7);
        CHECK(std::as_const(Binding)(5) == 9);
        auto Number = std::make_shared<int>(5);
        auto WeakNumber = std::weak_ptr<int>(Number);
        Retro::BindBack<add_to_shared_back>(std::move(Number))(4);
        CHECK(WeakNumber.expired());
    }

    SECTION("Binding two arguments works") {
        std::vector<int> Elements;
        auto Binding = Retro::BindBack<functor>(3, 4);
        CHECK(std::addressof(Elements) == std::addressof(Binding(Elements)));
        CHECK(Elements.size() == 2);
        std::as_const(Binding)(Elements);
        CHECK(Elements.size() == 4);
        Retro::BindBack<functor>(5, 6)(Elements);
        CHECK(Elements.size() == 6);
    }

    SECTION("Binding back to more than two arguments works") {
        auto Binding = Retro::BindBack<add_many>(4, 5, 6);
        CHECK(Binding(3) == 18);
        CHECK(std::as_const(Binding)(5) == 20);
        CHECK(Retro::BindBack<add_many>(10, 20, 30)(5) == 65);
    }
}

TEST_CASE("Can bind front to a constexpr defined functional type", "[functional]") {
    SECTION("Can bind with one parameter") {
        auto Binding = Retro::BindFront<&add_numbers>(1);
        CHECK(Binding(2, 3) == 6);
        CHECK(std::as_const(Binding)(5, 4) == 10);
        CHECK(Retro::BindFront<&add_numbers>(3)(5, 4) == 12);
    }

    SECTION("Can bind with two parameters") {
        auto Binding = Retro::BindFront<&add_numbers>(1, 2);
        CHECK(Binding(3) == 6);
        CHECK(std::as_const(Binding)(4) == 7);
        CHECK(Retro::BindFront<&add_numbers>(3, 6)(5) == 14);
    }

    SECTION("Can bind with three parameters") {
        auto Binding = Retro::BindFront<&add_numbers>(1, 2, 3);
        CHECK(Binding() == 6);
        CHECK(std::as_const(Binding)() == 6);
        CHECK(Retro::BindFront<&add_numbers>(3, 6, 9)() == 18);
    }
}

TEST_CASE("Can bind a method with an object at runtime", "[functional]") {
    SECTION("Can bind to an object of the given type") {
        TestClass Object;
        auto Binding = Retro::BindMethod(Object, &TestClass::method);
        CHECK(Binding(1, 2, 1) == 4);
    }

    SECTION("Can bind to a raw pointer of an object") {
        TestClass Object;
        auto Ptr = &Object;
        auto Binding = Retro::BindMethod(Ptr, &TestClass::method, 5);
        CHECK(Binding(4, 1) == 10);
        CHECK(std::as_const(Binding)(5, 5) == 15);
        CHECK(Retro::BindMethod(Ptr, &TestClass::method, 10)(5, 5) == 20);
    }

    SECTION("Can bind to wrapped pointer object") {
        auto Object = std::make_shared<TestClass>();
        auto Binding = Retro::BindMethod(Object, &TestClass::method, 5, 6);
        CHECK(Binding(4) == 15);
        CHECK(std::as_const(Binding)(5) == 16);
        CHECK(Retro::BindMethod(Object, &TestClass::method, 10, 12)(5) == 27);
    }

    SECTION("Can bind to a reference wrapper object") {
        TestClass Object;
        auto Binding = Retro::BindMethod(std::ref(Object), &TestClass::method, 5, 6, 4);
        CHECK(Binding() == 15);
        CHECK(std::as_const(Binding)() == 15);
        CHECK(Retro::BindMethod(std::cref(Object), &TestClass::method, 10, 12, 7)() == 29);
    }
}

TEST_CASE("Can bind a method with an object at compile time", "[functional]") {
    SECTION("Can bind to an object of the given type") {
        TestClass Object;
        auto Binding = Retro::BindMethod<&TestClass::method>(Object);
        CHECK(Binding(1, 2, 1) == 4);
    }

    SECTION("Can bind to a raw pointer of an object") {
        TestClass Object;
        auto Ptr = &Object;
        auto Binding = Retro::BindMethod<&TestClass::method>(Ptr, 5);
        CHECK(Binding(4, 1) == 10);
        CHECK(std::as_const(Binding)(5, 5) == 15);
        CHECK(Retro::BindMethod<&TestClass::method>(Ptr, 10)(5, 5) == 20);
    }

    SECTION("Can bind to wrapped pointer object") {
        auto object = std::make_shared<TestClass>();
        auto binding = Retro::BindMethod<&TestClass::method>(object, 5, 6);
        CHECK(binding(4) == 15);
        CHECK(std::as_const(binding)(5) == 16);
        CHECK(Retro::BindMethod<&TestClass::method>(object, 10, 12)(5) == 27);
    }

    SECTION("Can bind to a reference wrapper object") {
        TestClass Object;
        auto Binding = Retro::BindMethod<&TestClass::method>(std::reference_wrapper(Object), 5, 6, 4);
        CHECK(Binding() == 15);
        CHECK(std::as_const(Binding)() == 15);
        CHECK(Retro::BindMethod<&TestClass::method>(std::reference_wrapper(Object), 10, 12, 7)() == 29);
    }
}

TEST_CASE("Can use the opaque binding wrapper as runtime", "[functional]") {
    SECTION("Can bind a regular functor") {
        auto binding = Retro::CreateBinding(add, 4);
        CHECK(binding(3) == 7);
        CHECK(std::as_const(binding)(5) == 9);
        auto number = std::make_shared<int>(5);
        auto weakNumber = std::weak_ptr(number);
        Retro::CreateBinding(add_to_shared_back, std::move(number))(4);
        CHECK(weakNumber.expired());
    }

    SECTION("Can bind a method using the object as the owner, or bind back without it") {
        auto Object = std::make_shared<TestClass>();
        auto Binding = Retro::CreateBinding(Object, &TestClass::method, 5, 6);
        CHECK(Binding(4) == 15);
        CHECK(std::as_const(Binding)(5) == 16);
        CHECK(Retro::CreateBinding(&TestClass::method, 10, 12)(Object, 5) == 27);
    }

    SECTION("Can bind to a member") {
        TestClass Object;
        auto Binding1 = Retro::CreateBinding(Object, &TestClass::member);
        CHECK(Binding1() == 9);
        auto Binding2 = Retro::CreateBinding(&TestClass::member);
        CHECK(Binding2(Object) == 9);
    }

    SECTION("Can bind a functor and use tuples with it") {
        static_assert(Retro::TupleLike<std::pair<int, int>>);
        auto Binding = Retro::CreateBinding(add);
        CHECK(Binding(std::make_pair(3, 4)) == 7);
        CHECK(std::as_const(Binding)(std::make_pair(5, 4)) == 9);
        auto Number = std::make_shared<int>(5);
        auto WeakNumber = std::weak_ptr(Number);
        Retro::CreateBinding(add_to_shared_back)(std::make_pair(4, std::move(Number)));
        CHECK(WeakNumber.expired());
    }
}

TEST_CASE("Can use the opaque binding wrapper at compile time", "[functional]") {
    SECTION("Can bind a regular functor") {
        auto Binding = Retro::CreateBinding<add>(4);
        CHECK(Binding(3) == 7);
        CHECK(std::as_const(Binding)(5) == 9);
        auto Number = std::make_shared<int>(5);
        auto WeakNumber = std::weak_ptr(Number);
        Retro::CreateBinding<add_to_shared_back>(std::move(Number))(4);
        CHECK(WeakNumber.expired());
    }

    SECTION("Can bind a method using the this tag, or bind back without it") {
        auto Object = std::make_shared<TestClass>();
        auto Binding = Retro::CreateBinding<&TestClass::method>(Retro::This, Object, 5, 6);
        CHECK(Binding(4) == 15);
        CHECK(std::as_const(Binding)(5) == 16);
        CHECK(Retro::CreateBinding<&TestClass::method>(10, 12)(Object, 5) == 27);
    }

    SECTION("Can bind to a member") {
        TestClass Object;
        auto Binding1 = Retro::CreateBinding<&TestClass::member>(Object);
        CHECK(Binding1() == 9);
        auto Binding2 = Retro::CreateBinding<&TestClass::member>();
        CHECK(Binding2(Object) == 9);
    }

    SECTION("Can bind a functor and use tuples with it") {
        auto Binding = Retro::CreateBinding<add>();
        CHECK(Binding(std::make_pair(3, 4)) == 7);
        CHECK(std::as_const(Binding)(std::make_pair(5, 4)) == 9);
        auto Number = std::make_shared<int>(5);
        auto WeakNumber = std::weak_ptr(Number);
        Retro::CreateBinding<add_to_shared_back>()(std::make_pair(4, std::move(Number)));
        CHECK(WeakNumber.expired());
    }
}