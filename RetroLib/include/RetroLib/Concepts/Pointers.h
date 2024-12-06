/**
 * @file Pointers.h
 * @brief TODO: Fill me out
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/Optionals/OptionalOperations.h"
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro {
    template <typename T>
    concept PointerType = Dereferenceable<T> && !optionals::OptionalType<T> && requires(T&& ptr) {
        { std::forward<T>(ptr) == nullptr } -> std::same_as<bool>;
        { std::forward<T>(ptr) != nullptr } -> std::same_as<bool>;
    };
}