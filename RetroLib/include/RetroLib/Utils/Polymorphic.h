/**
 * @file Polymorphic.h
 * @brief Contains the declaration for the polymorphic class.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#ifndef RETROLIB_EXPORT
#include "RetroLib/Concepts/Inheritance.h"
#include "RetroLib/Utils/OpaqueStorage.h"
#include <typeinfo>

#define RETROLIB_EXPORT
#endif

namespace Retro {

    RETROLIB_EXPORT template <ClassType T>
    class Polymorphic {
      public:
        constexpr Polymorphic() noexcept requires std::is_default_constructible_v<T> : vtable(getVTable<T>()) {
            storage.emplace<T>();
        }

        template <typename U>
            requires std::derived_from<std::decay_t<U>, T>
        explicit constexpr Polymorphic(U&& value) noexcept : vtable(getVTable<U>()) {
            storage.emplace<std::decay_t<U>>(std::forward<U>(value));
        }

        template <typename U, typename A>
            requires std::derived_from<U, T> && std::constructible_from<U, A...>
        explicit constexpr Polymorphic(std::in_place_type_t<U>, A&&... args) noexcept : vtable(getVTable<U>()) {
            storage.emplace<U>(std::forward<A>(args)...);
        }

        constexpr Polymorphic(const Polymorphic& other) noexcept : vtable(other.vtable) {
            vtable->copy(other.storage, storage);
        }

        constexpr Polymorphic(Polymorphic&& other) noexcept : vtable(other.vtable) {
            vtable->move(other.storage, storage);
        }

        constexpr ~Polymorphic() noexcept {
            vtable->destroy(storage);
        }

        constexpr Polymorphic& operator=(const Polymorphic& other) noexcept {
            if (vtable->getType() == other.vtable->getType()) {
                vtable = other.vtable;
                vtable->copyAssign(other.storage, storage);
            } else {
                vtable->destroy(storage);
                vtable = other.vtable;
                vtable->copy(other.storage, storage);
            }

            return *this;
        }

        constexpr Polymorphic& operator=(const Polymorphic&& other) noexcept {
            if (vtable->getType() == other.vtable->getType()) {
                vtable = other.vtable;
                vtable->moveAssign(other.storage, storage);
            } else {
                vtable->destroy(storage);
                vtable = other.vtable;
                vtable->move(other.storage, storage);
            }
            return *this;
        }

        template <typename U>
            requires std::derived_from<std::decay_t<U>, T>
        constexpr Polymorphic& operator=(U&& value) noexcept {

        }

        constexpr T* get() {
            return vtable->getValue(storage);
        }

        constexpr const T* get() const {
            return vtable->getValue(storage);
        }

        constexpr T* operator->() {
            return get();
        }

        constexpr const T* operator->() const {
            return get();
        }

        constexpr T& operator*() {
            return *get();
        }

        constexpr const T& operator*() const {
            return *get();
        }

      private:
        OpaqueStorage storage;
        const VTable* vtable;
    };

} // Retro
