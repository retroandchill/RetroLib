/**
 * @file Variant.h
 * @brief TODO: Fill me out
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */

#pragma once

#if !RETROLIB_WITH_MODULES
#include <variant>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro {

    template <typename T, size_t I>
    struct IndexedElement {
    private:
        std::add_pointer_t<T> ptr;

    public:
        constexpr explicit IndexedElement(T& ptr) : ptr(std::addressof(ptr)) {}

        constexpr T& get() const noexcept { return *ptr; }
    };

    template <typename T, size_t I>
    struct IndexedElement<T&&, I> {
    private:
        T* ptr;

    public:
        constexpr explicit IndexedElement(T&& ptr) : ptr(std::addressof(ptr)) {}

        constexpr T&& get() const noexcept { return std::move(*ptr); }
    };

    template <size_t I>
    struct IndexedElement<void, I> {
        constexpr void get() const noexcept {
            // No operation needed for this one
        }
    };

}