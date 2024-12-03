/**
 * @file Optional.h
 * @brief Custom optional type that has support for reference-based optionals.
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

    template <typename>
    struct IsValidOptionalType : std::true_type {};

    template <>
    struct IsValidOptionalType<std::nullopt_t> : std::false_type {};

    template <typename T>
    struct IsValidOptionalType<std::in_place_type_t<T>> : std::false_type {};

    template <typename T>
    concept ValidOptionalType = IsValidOptionalType<std::decay_t<T>>::value;

    template <ValidOptionalType T>
    struct OptionalStorage {
        constexpr OptionalStorage() noexcept = default;

        constexpr OptionalStorage(const OptionalStorage& other) requires std::is_copy_constructible_v<T> : is_set(other.is_set) {
            if (is_set) {
                new (&value) T(other.value);
            }
        }

        constexpr OptionalStorage(OptionalStorage&& other) noexcept(std::is_nothrow_move_constructible_v<T>) requires std::is_move_constructible_v<T> : is_set(other.is_set) {
            if (is_set) {
                new (&value) T(std::move(other.value));
            }
        }

        template <typename U>
            requires std::constructible_from<T, const U&>
        constexpr explicit(!std::convertible_to<const U&, T>) OptionalStorage(const OptionalStorage<U>& other) : is_set(other.is_set) {
            if (is_set) {
                new (&value) T(other.value);
            }
        }

        template <typename U>
            requires std::constructible_from<T, U&&>
        constexpr explicit(!std::convertible_to<U&&, T>) OptionalStorage(OptionalStorage<U>&& other) : is_set(other.is_set) {
            if (is_set) {
                new (&value) T(std::move(other.value));
            }
        }

        template <typename... A>
            requires std::constructible_from<T, A...>
        constexpr explicit OptionalStorage(std::in_place_type_t<T>, A&&... args) : value(std::forward<A>(args)...), is_set(true) {
        }

        template <typename U, typename... A>
            requires std::constructible_from<T, std::initializer_list<U>, A...>
        constexpr explicit OptionalStorage(std::in_place_type_t<T>, std::initializer_list<U> initializer_list, A&&... args)  : value(std::move(initializer_list), std::forward<A>(args)...), is_set(true) {}

        template <typename U = T>
            requires std::constructible_from<T, U>
        constexpr explicit(std::convertible_to<U, T>) OptionalStorage(U&& value) : value(std::forward<U>(value)), is_set(true) {
        }

        ~OptionalStorage() {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                if (is_set) {
                    value.~T();
                }
            }
        }

        constexpr OptionalStorage& operator=(const OptionalStorage& other) requires std::is_copy_assignable_v<T> {
            if (is_set && other.is_set) {
                value = other.value;
            } else if (other.is_set) {
                new (&value) T(other.value);
            }  else {
                if constexpr (!std::is_trivially_copyable_v<T>) {
                    if (is_set) {
                        value.~T();
                    }
                }
            }

            is_set = other.is_set;
            return *this;
        }

        constexpr OptionalStorage& operator=(OptionalStorage&& other) noexcept(std::is_nothrow_move_assignable_v<T>) requires std::is_move_assignable_v<T> {
            if (is_set && other.is_set) {
                value = std::move(other.value);
            } else if (other.is_set) {
                new (&value) T(std::move(other.value));
            } else {
                if constexpr (!std::is_trivially_copyable_v<T>) {
                    if (is_set) {
                        value.~T();
                    }
                }
            }

            is_set = other.is_set;
            return *this;
        }

        template <typename U = T>
            requires std::assignable_from<T, U>
        constexpr OptionalStorage& operator=(U&& other) {
            if (is_set) {
                value = std::forward<U>(other);
            } else {
                new (&value) T(std::forward<U>(other));
            }

            is_set = other.is_set;
            return *this;
        }

        template <typename U = T>
            requires std::assignable_from<T, const U&>
        constexpr OptionalStorage& operator=(const OptionalStorage<U>& other) {
            if (is_set && other.is_set) {
                value = other.value;
            } else if (other.is_set) {
                new (&value) T(other.value);
            }  else {
                if constexpr (!std::is_trivially_copyable_v<T>) {
                    if (is_set) {
                        value.~T();
                    }
                }
            }

            is_set = other.is_set;
            return *this;
        }

        template <typename U = T>
            requires std::assignable_from<T, U&&>
        constexpr OptionalStorage& operator=(OptionalStorage<U>&& other) {
            if (is_set && other.is_set) {
                value = other.value;
            } else if (other.is_set) {
                new (&value) T(other.value);
            }  else {
                if constexpr (!std::is_trivially_copyable_v<T>) {
                    if (is_set) {
                        value.~T();
                    }
                }
            }

            is_set = other.is_set;
            return *this;
        }

        constexpr T& get_value() & {
            return value;
        }

        constexpr const T& get_value() const & {
            return value;
        }

        constexpr T&& get_value() && {
            return std::move(value);
        }

        constexpr bool has_value() const {
            return is_set;
        }

        constexpr void swap(OptionalStorage& other) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_swappable_v<T>) {
            if (!is_set && !other.is_set) {
                return;
            }

            if (is_set && !other.is_set) {
                other.value = std::move(value);
                value.~T();
                is_set = false;
                other.is_set = true;
            } else if (!is_set) {
                value = std::move(other.value);
                other.value.~T();
                other.is_set = false;
                is_set = true;
            } else {
                std::swap(value, other.value);
            }
        }

        constexpr void reset() {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                if (has_value()) {
                    value.~T();
                }
            }

            is_set = false;
        }

        template <typename... A>
            requires std::constructible_from<T, A...>
        constexpr void emplace(A&&... args) {
            if (is_set) {
                value = T(std::forward<A>(args)...);
            } else {
                new (&value) T(std::forward<A>(args)...);
            }

            is_set = true;
        }

        template <typename U, typename... A>
            requires std::constructible_from<T, std::initializer_list<U>, A...>
        constexpr void emplace(std::initializer_list<U> initializer_list, A&&... args) {
            if (is_set) {
                value = T(std::move(initializer_list), std::forward<A>(args)...);
            } else {
                new (&value) T(std::move(initializer_list), std::forward<A>(args)...);
            }

            is_set = true;
        }

        template <typename U>
            requires std::equality_comparable_with<const T&, const U&>
        constexpr bool operator==(const OptionalStorage<U>& other) const {
            if (!is_set && !other.is_set) {
                return true;
            }

            if (is_set && other.is_set) {
                return value == other.value;
            }

            return false;
        }

        template <typename U>
            requires std::three_way_comparable_with<const T&, const U&>
        constexpr std::strong_ordering operator<=>(const OptionalStorage<U>& other) const {
            if (!is_set && !other.is_set) {
                return std::strong_ordering::equal;
            }

            if (is_set && other.is_set) {
                return value <=> other.value;
            }

            if (is_set && !other.is_set) {
                return std::strong_ordering::greater;
            }

            return std::strong_ordering::less;
        }

        template <typename U>
            requires std::equality_comparable_with<const T&, const U&>
        constexpr bool operator==(const U& other) const {
            if (!is_set) {
                return false;
            }

            return value == other;
        }

        template <typename U>
            requires std::three_way_comparable_with<const T&, const U&>
        constexpr std::strong_ordering operator<=>(const U& other) const {
            if (!is_set) {
                return std::strong_ordering::less;
            }

            return value <=> other;
        }

    private:
        union {
            std::nullopt_t empty;
            T value;
        };
        bool is_set = false;
    };

    template <ValidOptionalType T>
    class Optional {
    public:
        constexpr Optional() noexcept = default;
        constexpr explicit Optional(std::nullopt_t) noexcept {}

        constexpr Optional(const Optional&) requires std::is_copy_constructible_v<T> = default;
        constexpr Optional(Optional&&)noexcept(std::is_nothrow_move_constructible_v<T>) requires std::is_move_constructible_v<T> = default;

        template <typename U>
            requires std::constructible_from<T, const U&>
        constexpr explicit(!std::convertible_to<const U&, T>) Optional(const Optional<U>& other) : storage(other.storage) {}

        template <typename U>
            requires std::constructible_from<T, U&&>
        constexpr explicit(!std::convertible_to<U&&, T>) Optional(Optional<U>&& other) : storage(std::move(other.storage)) {}

        template <typename... A>
            requires std::constructible_from<T, A...>
        constexpr explicit Optional(std::in_place_type_t<T>, A&&... args) : storage(std::in_place_type<T>, std::forward<A>(args)...) {}

        template <typename U, typename... A>
            requires std::constructible_from<T, std::initializer_list<U>, A...>
        constexpr explicit Optional(std::in_place_type_t<T>, std::initializer_list<U> initializer_list, A&&... args) : storage(std::in_place_type<T>, std::move(initializer_list), std::forward<A>(args)...) {}

        template <typename U = T>
            requires std::constructible_from<T, U>
        constexpr explicit(std::convertible_to<U, T>) Optional(U&& value) : storage(std::forward<U>(value)) {}

        ~Optional() = default;

        constexpr Optional& operator=(const Optional&) requires std::is_copy_assignable_v<T> = default;
        constexpr Optional& operator=(Optional&&) noexcept(std::is_nothrow_move_assignable_v<T>) requires std::is_move_assignable_v<T> = default;

        constexpr Optional& operator=(std::nullopt_t) {
            storage.reset();
            return *this;
        }

        template <typename U = T>
            requires std::convertible_to<U, T>
        constexpr Optional& operator=(U&& value) {
            storage = std::forward<U>(value);
            return *this;
        }

        template <typename U = T>
            requires std::convertible_to<const U&, T>
        constexpr Optional& operator=(const Optional<U>& other) {
            storage = other.storage;
            return *this;
        }

        template <typename U = T>
            requires std::convertible_to<U&&, T>
        constexpr Optional& operator=(Optional<U>&& other) {
            storage = std::move(other.storage);
            return *this;
        }

        constexpr decltype(auto) operator*() const & {
            return storage.get_value();
        }

        constexpr decltype(auto) operator*() const & {
            return storage.get_value();
        }

        constexpr decltype(auto) operator*() && {
            return std::move(storage.get_value());
        }

        constexpr decltype(auto) operator->() {
            return &storage.get_value();
        }

        constexpr decltype(auto) operator->() const {
            return &storage.get_value();
        }

        constexpr bool has_value() const {
            return storage.has_value();
        }

        constexpr decltype(auto) value() & {
            if (!has_value()) {
                throw std::bad_optional_access();
            }

            return storage.get_value();
        }

        constexpr decltype(auto) value() const & {
            if (!has_value()) {
                throw std::bad_optional_access();
            }

            return storage.get_value();
        }

        constexpr decltype(auto) value() && {
            if (!has_value()) {
                throw std::bad_optional_access();
            }

            return std::move(storage.get_value());
        }

        template <typename U>
            requires std::convertible_to<U, T>
        constexpr T value_or(U&& default_value) const & {
            if (has_value()) {
                return storage.get_value();
            }

            return std::forward<U>(default_value);
        }

        template <typename U>
            requires std::convertible_to<U, T>
        constexpr T value_or(U&& default_value) && {
            if (has_value()) {
                return std::move(storage.get_value());
            }

            return std::forward<U>(default_value);
        }

        constexpr void swap(Optional& other) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_swappable_v<T>) {
            storage.swap(other.storage);
        }

        constexpr void reset() noexcept {
            storage.reset();
        }

        template <typename... A>
            requires std::constructible_from<T, A...>
        constexpr void emplace(A&& args) {
            storage.emplace(std::forward<A>(args)...);
        }

        template <typename U, typename... A>
            requires std::constructible_from<T, std::initializer_list<U>, A...>
        constexpr void emplace(std::initializer_list<U> initializer_list, A&& args) {
            storage.emplace(std::move(initializer_list), std::forward<A>(args)...);
        }

        template <typename U>
			requires std::equality_comparable_with<const T&, const U&>
        constexpr bool operator==(const Optional<U>& other) const {
            return storage == other.storage;
        }

        template <typename U>
			requires std::three_way_comparable_with<const T&, const U&>
        constexpr std::strong_ordering operator<=>(const Optional<U>& other) const {
            return storage <=> other.storage;
        }

        constexpr bool operator==(std::nullopt_t) const {
            return !has_value();
        }

        constexpr std::strong_ordering operator<=>(std::nullopt_t) const {
            return has_value() ? std::strong_ordering::greater : std::strong_ordering::equal;
        }

        template <typename U>
            requires std::equality_comparable_with<const T&, const U&>
        constexpr bool operator==(const U& other) const {
            return storage == other;
        }

        template <typename U>
            requires std::three_way_comparable_with<const T&, const U&>
        constexpr std::strong_ordering operator<=>(const U& other) const {
            return storage <=> other;
        }

    private:
        OptionalStorage<T> storage;
    };
}