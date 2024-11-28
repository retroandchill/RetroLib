/**
 * @file Inheritance.h
 * @brief Contains concept definitions for polymorphic classes.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#ifndef RETROLIB_EXPORT
#include <type_traits>

#define RETROLIB_EXPORT
#endif

namespace Retro {

    RETROLIB_EXPORT template <typename T>
    concept PolymorphicType = std::is_class_v<T> && std::has_virtual_destructor_v<T>;

}
