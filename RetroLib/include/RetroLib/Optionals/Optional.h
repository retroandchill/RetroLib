/**
 * @file Optional.h
 * @brief Custom optional type that has support for reference-based optionals.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/RetroLibMacros.h"
#include "RetroLib/Concepts/ParameterPacks.h"
#include "RetroLib/Optionals/OptionalOperations.h"
#include "RetroLib/Concepts/Comparisons.h"

#include <utility>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro {
    /**
     * @struct IsValidOptionalType
     * @brief A type trait used to determine if a type is a valid optional type.
     *
     * This struct inherits from std::true_type, indicating that the default
     * case for this trait is true. It provides a mechanism to validate
     * types as optional, allowing for compile-time checks.
     *
     * Users can extend or specialize this struct to modify the behavior
     * for specific types, ensuring that custom types can be identified as
     * valid optional types when necessary.
     *
     * @note Specialize this struct for any custom type that should be
     *       considered as a valid optional type in your implementation.
     */
    template<typename>
    struct IsValidOptionalType : std::true_type {
    };

    /**
     * @struct IsValidOptionalType
     *
     * @brief Trait class template specialization to determine the validity of a type as an optional.
     *
     * This specialization of the `IsValidOptionalType` template is specifically for handling
     * `std::nullopt_t`, a type used to represent disengaged states in optional objects.
     * For this specialization, it derives from `std::false_type`, indicating that
     * `std::nullopt_t` is not a valid type for an optional object in this context.
     *
     * @tparam std::nullopt_t Specialized parameter representing the type indicating
     *         an uninitialized or empty state in optional objects.
     */
    template<>
    struct IsValidOptionalType<std::nullopt_t> : std::false_type {
    };

    /**
     * @brief Struct template specialization to define the validity of an optional type.
     *
     * This specialization of the `IsValidOptionalType` struct template is used to
     * mark `std::in_place_type_t<T>` as an invalid optional type. It inherits from
     * `std::false_type`, effectively indicating a logical "false" for checks that
     * rely on this struct to determine type validity.
     *
     * @tparam T The type encapsulated by `std::in_place_type_t`.
     */
    template<typename T>
    struct IsValidOptionalType<std::in_place_type_t<T> > : std::false_type {
    };

    /**
     * Concept to check if a type is allowed to be used in an optional.
     *
     * @tparam T The target type
     */
    RETROLIB_EXPORT template<typename T>
    concept ValidOptionalType = IsValidOptionalType<std::decay_t<T> >::value && std::destructible<T> &&
                                (std::is_object_v<T> || std::is_lvalue_reference_v<T>);

    RETROLIB_EXPORT template<ValidOptionalType T>
    class Optional;


    template<typename = void>
    [[noreturn]] bool throw_bad_optional_access() {
        throw std::bad_optional_access();
    }

    template<ValidOptionalType T>
    struct OptionalStorage {
        union {
            std::nullopt_t empty;
            std::remove_cv_t<T> data;
        };

        bool is_set = false;

        constexpr OptionalStorage() noexcept : empty(std::nullopt) {
        }

        template<typename... A>
            requires std::constructible_from<T, A...> && (!PackSameAs<OptionalStorage, A...>)
        constexpr explicit OptionalStorage(std::in_place_type_t<T>,
                                           A &&... args) noexcept(std::is_nothrow_constructible_v<T, A...>
        ) : data(std::forward<A>(args)...), is_set(true) {
        }

        constexpr void reset() noexcept {
            is_set = false;
        }
    };

    template<ValidOptionalType T>
        requires (!std::is_trivially_destructible_v<T>)
    struct OptionalStorage<T> {
        union {
            std::nullopt_t empty;
            std::remove_cv_t<T> data;
        };

        bool is_set = false;

        constexpr OptionalStorage() noexcept : empty(std::nullopt) {
        }

        template<typename... A>
            requires std::constructible_from<T, A...> && (!PackSameAs<std::decay_t<OptionalStorage>, A...>)
        constexpr explicit OptionalStorage(std::in_place_type_t<T>,
                                           A &&... args) noexcept(std::is_nothrow_constructible_v<T, A...>
        ) : data(std::forward<A>(args)...), is_set(true) {
        }


        constexpr OptionalStorage(const OptionalStorage &other) = default;

        constexpr OptionalStorage(OptionalStorage &&other) = default;

        ~OptionalStorage() {
            reset();
        }

        constexpr OptionalStorage &operator=(const OptionalStorage &other) = default;

        constexpr OptionalStorage &operator=(OptionalStorage &&other) = default;

        constexpr void reset() noexcept {
            if (is_set) {
                data.~T();
                is_set = false;
            }
        }
    };

    template<ValidOptionalType T>
    struct OptionalBase : private OptionalStorage<T> {
        using OptionalStorage<T>::OptionalStorage;
        using OptionalStorage<T>::reset;

        OptionalBase() noexcept = default;

        constexpr bool has_value() const noexcept {
            return is_set;
        }

        constexpr T &operator*() & noexcept {
            RETROLIB_ASSERT(is_set);
            return data;
        }

        constexpr const T &operator*() const & noexcept {
            RETROLIB_ASSERT(is_set);
            return data;
        }

        constexpr const T &operator*() && noexcept {
            RETROLIB_ASSERT(is_set);
            return data;
        }

        constexpr T *operator->() & noexcept {
            RETROLIB_ASSERT(is_set);
            return &data;
        }

        constexpr const T *operator->() const & noexcept {
            RETROLIB_ASSERT(is_set);
            return &data;
        }

        constexpr void swap(
            OptionalBase &other) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_swappable_v<T>)
            requires std::swappable<T> {
            constexpr bool can_swap_trivially = std::is_trivially_move_constructible_v<T> &&
                                                std::is_trivially_move_assignable_v<T>;
            if constexpr (can_swap_trivially) {
                std::swap(static_cast<OptionalStorage<T> &>(*this), static_cast<OptionalStorage<T> &>(other));
            } else {
                if (is_set == other.is_set) {
                    if (is_set) {
                        std::swap(data, other.data);
                    }
                } else {
                    auto &src = is_set ? data : other.data;
                    auto &dst = is_set ? other.data : data;
                    dst.construct_from(std::move(src.data));
                    src.reset();
                }
            }
        }

    protected:
        template<typename... A>
            requires std::constructible_from<T, A...>
        T &construct_from(A &&... args) noexcept(std::is_nothrow_constructible_v<T, A...>) {
            RETROLIB_ASSERT(!is_set);
            new(&data) T(std::forward<A>(args)...);
            is_set = true;
            return data;
        }

        template<typename I>
            requires std::constructible_from<T, I>
        T &construct_from_deref(const I &it) {
            RETROLIB_ASSERT(!is_set);
            new(&data) T(*it);
            is_set = true;
            return data;
        }

        template<typename U>
        constexpr void assign_from(
            U &&other) noexcept(std::is_nothrow_constructible_v<T, decltype(*std::forward<U>(other))> &&
                                std::is_nothrow_assignable_v<T &, decltype(*std::forward<U>(other))>) {
            if (!other.has_value()) {
                reset();
            } else if (is_set) {
                data = std::forward<U>(other).data;
            } else {
                new(&data) T(*std::forward<U>(other));
                is_set = true;
            }
        }

    private:
        using OptionalStorage<T>::data;
        using OptionalStorage<T>::is_set;
    };

    template<ValidOptionalType T>
    struct OptionalBase<T &> {
        OptionalBase() noexcept = default;

        template<typename A>
            requires std::constructible_from<T &, A>
        constexpr explicit OptionalBase(std::in_place_type_t<T &>, A &&arg) noexcept : data(&arg) {
        }

        constexpr bool has_value() const noexcept {
            return data != nullptr;
        }

        constexpr T &operator*() noexcept {
            RETROLIB_ASSERT(data != nullptr);
            return *data;
        }

        constexpr const T &operator*() const noexcept {
            RETROLIB_ASSERT(data != nullptr);
            return *data;
        }

        constexpr T *operator->() noexcept {
            RETROLIB_ASSERT(data != nullptr);
            return data;
        }

        constexpr const T *operator->() const noexcept {
            RETROLIB_ASSERT(data != nullptr);
            return data;
        }

        constexpr void reset() noexcept {
            data = nullptr;
        }

        constexpr void swap(OptionalBase &other) noexcept(std::is_nothrow_swappable_v<T>) requires std::swappable<T> {
            if (data != nullptr && other.data != nullptr) {
                std::swap(*data, *other.data);
            } else {
                std::swap(data, other.data);
            }
        }

    protected:
        template<typename U>
            requires std::convertible_to<U &, T &>
        constexpr T &construct_from(U &&ref) noexcept {
            RETROLIB_ASSERT(data == nullptr);
            data = &ref;
            return *data;
        }

        template<typename U>
        constexpr void assign_from(U &&other) {
            if (data != nullptr && other.data != nullptr) {
                *data = *std::forward<U>(other).data;
            } else {
                data = std::forward<U>(other).data;
            }
        }

    private:
        T *data = nullptr;
    };

    template<ValidOptionalType T>
    struct OptionalCopy : OptionalBase<T> {
        OptionalCopy() noexcept = default;

        OptionalCopy(const OptionalCopy &other) noexcept(std::is_nothrow_copy_constructible_v<T>) {
            if (other.has_value()) {
                this->construct_from(*other);
            }
        }

        OptionalCopy(OptionalCopy &&) = default;

        ~OptionalCopy() = default;

        OptionalCopy &operator=(const OptionalCopy &) = default;

        OptionalCopy &operator=(OptionalCopy &&) = default;

        using OptionalBase<T>::OptionalBase;
    };

    template<ValidOptionalType T>
    using CopyConstructLayer = std::conditional_t<
        std::is_copy_constructible_v<T> && !std::is_trivially_copy_constructible_v<T>,
        OptionalCopy<T>, OptionalBase<T> >;

    template<ValidOptionalType T>
    struct OptionalMove : CopyConstructLayer<T> {
        OptionalMove() noexcept = default;

        OptionalMove(const OptionalMove &) = default;

        OptionalMove(OptionalMove &&other) noexcept(std::is_nothrow_move_constructible_v<T>) {
            if (other.has_value()) {
                this->construct_from(std::move(*other));
            }
        }

        ~OptionalMove() = default;

        OptionalMove &operator=(const OptionalMove &) = default;

        OptionalMove &operator=(OptionalMove &&) = default;

        using CopyConstructLayer<T>::CopyConstructLayer;
    };

    template<ValidOptionalType T>
    using MoveConstructLayer = std::conditional_t<std::is_move_constructible_v<T> &&
                                                  !std::is_trivially_move_constructible_v<T>,
        OptionalMove<T>, CopyConstructLayer<T> >;

    template<ValidOptionalType T>
    struct OptionalCopyAssign : MoveConstructLayer<T> {
        OptionalCopyAssign() noexcept = default;

        OptionalCopyAssign(const OptionalCopyAssign &) = default;

        OptionalCopyAssign(OptionalCopyAssign &&) = default;

        ~OptionalCopyAssign() = default;

        OptionalCopyAssign &operator=(
            const OptionalCopyAssign &other) noexcept(
            std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_constructible_v<T, decltype(*other)>) {
            this->assign_from(other);
            return *this;
        }

        OptionalCopyAssign &operator=(OptionalCopyAssign &&) = default;

        using MoveConstructLayer<T>::MoveConstructLayer;
    };

    template<ValidOptionalType T>
    struct DeletedCopyAssign : MoveConstructLayer<T> {
        DeletedCopyAssign() noexcept = default;

        DeletedCopyAssign(const DeletedCopyAssign &) = default;

        DeletedCopyAssign(DeletedCopyAssign &&) = default;

        ~DeletedCopyAssign() = default;

        DeletedCopyAssign &operator=(const DeletedCopyAssign &) = delete;

        DeletedCopyAssign &operator=(DeletedCopyAssign &&) = default;

        using MoveConstructLayer<T>::MoveConstructLayer;
    };

    template<ValidOptionalType T>
    using CopyAssignLayer = std::conditional_t<std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>,
        std::conditional_t<std::is_reference_v<T> ||
                           !(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T>),
            OptionalCopyAssign<T>, MoveConstructLayer<T> >, DeletedCopyAssign<T> >;

    template<ValidOptionalType T>
    struct OptionalMoveAssign : CopyAssignLayer<T> {
        OptionalMoveAssign() noexcept = default;

        OptionalMoveAssign(const OptionalMoveAssign &) = default;

        OptionalMoveAssign(OptionalMoveAssign &&other) = default;

        ~OptionalMoveAssign() = default;

        OptionalMoveAssign &operator=(const OptionalMoveAssign &other) = default;

        OptionalMoveAssign &operator=(
            OptionalMoveAssign &&other) noexcept(
            std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>) {
            this->assign_from(std::move(other));
            return *this;
        }

        using CopyAssignLayer<T>::CopyAssignLayer;
    };

    template<ValidOptionalType T>
    struct DeletedMoveAssign : CopyAssignLayer<T> {
        DeletedMoveAssign() noexcept = default;

        DeletedMoveAssign(const DeletedMoveAssign &) = default;

        DeletedMoveAssign(DeletedMoveAssign &&) = default;

        ~DeletedMoveAssign() = default;

        DeletedMoveAssign &operator=(const DeletedMoveAssign &) = default;

        DeletedMoveAssign &operator=(DeletedMoveAssign &&) = delete;

        using CopyAssignLayer<T>::CopyAssignLayer;
    };

    template<ValidOptionalType T>
    using MoveAssignLayer = std::conditional_t<std::is_move_constructible_v<T> && std::is_move_assignable_v<T>,
        std::conditional_t<std::is_reference_v<T> ||
                           !(std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T>),
            OptionalMoveAssign<T>, CopyAssignLayer<T> >, DeletedMoveAssign<T> >;

    template<typename U, typename T>
    concept OptionalShouldConvert = !(std::constructible_from<T, Optional<U> &> ||
                                      std::constructible_from<T, Optional<U> &&> ||
                                      std::constructible_from<T, const Optional<U> &> ||
                                      std::constructible_from<T, const Optional<U> &&> ||
                                      std::convertible_to<Optional<U> &, T> ||
                                      std::convertible_to<Optional<U> &&, T> ||
                                      std::convertible_to<const Optional<U> &, T> ||
                                      std::convertible_to<const Optional<U> &&, T>);

    template<typename U, typename T>
    concept OptionalShouldConvertAssign = OptionalShouldConvert<U, T> ||
                                          !(std::assignable_from<T &, Optional<U> &> ||
                                            std::assignable_from<T &, Optional<U> &&> ||
                                            std::assignable_from<T &, const Optional<U> &> ||
                                            std::assignable_from<T &, const Optional<U> &&>);

    template<ValidOptionalType T>
    class Optional : private MoveAssignLayer<T> {
        using Base = MoveAssignLayer<T>;

    public:
        using ValueType = std::conditional_t<std::is_lvalue_reference_v<T>, T, std::remove_cv_t<T> >;


        constexpr Optional() noexcept = default;

        constexpr explicit(false) Optional(std::nullopt_t) noexcept : Base() {
        }

        template<ValidOptionalType U = T>
            requires std::constructible_from<T, U> && (!std::same_as<std::decay_t<U>, Optional>)
        constexpr explicit(!std::convertible_to<U, T>) Optional(U &&value) : Base(
            std::in_place_type<T>, std::forward<U>(value)) {
        }

        Optional(const Optional &) = default;

        Optional(Optional &&) = default;

        using Base::Base;

        template<OptionalShouldConvert<T> U>
            requires std::constructible_from<T, const U &>
        constexpr explicit(!std::convertible_to<const U &, T>) Optional(const Optional<U> &other) {
            if (other.has_value()) {
                Base::construct_from(*other);
            }
        }


        template<OptionalShouldConvert<T> U>
            requires std::constructible_from<T, U &&>
        constexpr explicit(!std::convertible_to<const U &, T>) Optional(Optional<U> &&other) {
            if (other.has_value()) {
                Base::construct_from(std::move(*other));
            }
        }

        ~Optional() = default;

        constexpr Optional &operator=(std::nullopt_t) noexcept {
            reset();
            return *this;
        }

        constexpr Optional &operator=(const Optional &other) = default;

        constexpr Optional &operator=(Optional &&other) = default;

        template<ValidOptionalType U = T>
            requires (!std::same_as<std::decay_t<U>, Optional> && std::constructible_from<T, U> && std::assignable_from<
                          T &, U>)
        constexpr Optional &operator=(
            U &&other) noexcept(std::is_nothrow_constructible_v<T, U> && std::is_nothrow_assignable_v<T &, U>) {
            if (has_value()) {
                **this = std::forward<U>(other);
            } else {
                Base::construct_from(std::forward<U>(other));
            }

            return *this;
        }

        template<OptionalShouldConvertAssign<T> U>
            requires std::constructible_from<T, const U &> && std::assignable_from<T &, const U &>
        constexpr Optional &operator=(const Optional<U> &other) {
            Base::assign_from(other);
            return *this;
        }

        template<OptionalShouldConvertAssign<T> U>
            requires std::constructible_from<T, U &&> && std::assignable_from<T &, U &&>
        constexpr Optional &operator=(Optional<U> &&other) {
            Base::assign_from(std::move(other));
            return *this;
        }

        template<typename I>
            requires std::constructible_from<T, decltype(*std::declval<const I &>())>
        T &emplace_deref(const I &it) {
            reset();
            return Base::construct_from_deref(it);
        }

        template<typename... A>
            requires std::constructible_from<T, A...>
        T &emplace(A &&... args) noexcept(std::is_nothrow_constructible_v<T, A...>) {
            reset();
            return Base::construct_from(std::forward<A>(args)...);
        }

        template<typename U, typename... A>
            requires std::constructible_from<T, std::initializer_list<U>, A...>
        T &emplace(std::initializer_list<U> initializer_list,
                   A &&... args) noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>, A...>) {
            reset();
            return Base::construct_from(initializer_list, std::forward<A>(args)...);
        }

        using Base::swap;
        using Base::operator->;
        using Base::operator*;

        using Base::has_value;

        constexpr T &value() & {
            return has_value() || throw_bad_optional_access(), **this;
        }

        constexpr const T &value() const & {
            return has_value() || throw_bad_optional_access(), **this;
        }

        constexpr T &&value() && {
            return has_value() || throw_bad_optional_access(), std::move(**this);
        }

        constexpr const T &&value() const && {
            return has_value() || throw_bad_optional_access(), std::move(**this);
        }

        template<ValidOptionalType U>
            requires std::copy_constructible<T> && std::convertible_to<U, T>
        constexpr T value_or(U &&default_value) const & {
            return has_value() ? **this : std::forward<U>(default_value);
        }

        template<ValidOptionalType U>
            requires std::move_constructible<U> && std::convertible_to<U, T>
        constexpr T value_or(U &&default_value) && {
            return has_value() ? std::move(**this) : std::forward<U>(default_value);
        }

        using Base::reset;
    };

    RETROLIB_EXPORT template<ValidOptionalType T, ValidOptionalType U>
    constexpr bool operator==(const Optional<T> &self, const Optional<U> &other) noexcept(noexcept(*self == *other)) {
        return self.has_value() == other.has_value() && (!self.has_value() || *self == *other);
    }

    RETROLIB_EXPORT template<ValidOptionalType T, ValidOptionalType U>
    constexpr auto operator<=>(const Optional<T> &self, const Optional<U> &other) noexcept(noexcept(*self <=> *other)) {
        using ResultType = std::compare_three_way_result_t<T, U>;
        if (self.has_value() && !other.has_value()) {
            return ResultType::greater;
        }

        if (!self.has_value() && other.has_value()) {
            return ResultType::less;
        }

        if (!self.has_value() && !other.has_value()) {
            return ResultType::equivalent;
        }

        return *self <=> *other;
    }

    RETROLIB_EXPORT template<ValidOptionalType T>
    constexpr bool operator==(const Optional<T> &self, std::nullopt_t) noexcept {
        return !self.has_value();
    }

    RETROLIB_EXPORT template<ValidOptionalType T>
    constexpr std::strong_ordering operator<=>(const Optional<T> &self, std::nullopt_t) noexcept {
        return self.has_value() ? std::strong_ordering::greater : std::strong_ordering::equal;
    }

    RETROLIB_EXPORT template<ValidOptionalType T>
    constexpr bool operator==(std::nullopt_t, const Optional<T> &other) noexcept {
        return !other.has_value();
    }

    RETROLIB_EXPORT template<ValidOptionalType T>
    constexpr std::strong_ordering operator<=>(std::nullopt_t, const Optional<T> &other) noexcept {
        return other.has_value() ? std::strong_ordering::less : std::strong_ordering::equal;
    }

    RETROLIB_EXPORT template <ValidOptionalType T, typename U>
    constexpr bool operator==(const Optional<T>& self, const U &other) noexcept(noexcept(*self == other)) {
        return self.has_value() && *self == other;
    }

    RETROLIB_EXPORT template <ValidOptionalType T, typename U>
    constexpr auto operator<=>(const Optional<T>& self, const U &other) noexcept(noexcept(*self <=> other)) {
        using ResultType = std::compare_three_way_result_t<T, U>;
        return self.has_value() ? *self <=> other : ResultType::greater;
    }

    RETROLIB_EXPORT template <ValidOptionalType T, typename U>
    constexpr bool operator==(const U &other, const Optional<T> &self) noexcept(noexcept(other == *self)) {
        return self.has_value() && other == *self;
    }

    RETROLIB_EXPORT template <ValidOptionalType T, typename U>
    constexpr auto operator<=>(const U &other, const Optional<T> &self) noexcept(noexcept(other <=> *self)) {
        using ResultType = std::compare_three_way_result_t<T, U>;
        return self.has_value() ? other <=> *self : ResultType::less;
    }

    template<ValidOptionalType T>
    Optional(T) -> Optional<T>;
}
