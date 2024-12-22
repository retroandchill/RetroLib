/**
 * @file ForEachRange.h
 * @brief Basic iterator definitions to allow a container that can be used for a range-based for-loop to work with C++20
 * ranges.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/Ranges/Concepts/Iterable.h"
#include "RetroLib/TypeTraits.h"

#include <ranges>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro::ranges {

    template <typename I>
    struct IteratorStorage {
        I adapted;

        constexpr IteratorStorage() = default;

        template <typename T>
            requires std::constructible_from<I, T> && (!std::same_as<std::decay_t<T>, IteratorStorage>)
        constexpr explicit IteratorStorage(T &&adapted) : adapted(std::forward<T>(adapted)) {
        }
    };

    template <typename I>
        requires(!std::is_default_constructible_v<I>)
    struct IteratorStorage<I> {
        union {
            std::monostate empty;
            I adapted;
        };

        constexpr IteratorStorage() : empty() {
        }

        template <typename T>
            requires std::constructible_from<I, T> && (!std::same_as<std::decay_t<T>, IteratorStorage>)
        constexpr explicit IteratorStorage(T &&adapted) : adapted(std::forward<T>(adapted)) {
        }
    };

    template <typename I>
        requires std::is_trivially_destructible_v<I> && std::is_trivially_copy_constructible_v<I> &&
                 std::is_trivially_move_constructible_v<I>
    struct IteratorAssignAdapter : IteratorStorage<I> {
        constexpr IteratorAssignAdapter() = default;

        constexpr IteratorAssignAdapter(const IteratorAssignAdapter &) noexcept(
            std::is_nothrow_copy_constructible_v<I>) = default;
        constexpr IteratorAssignAdapter(IteratorAssignAdapter &&) noexcept(std::is_nothrow_move_constructible_v<I>) =
            default;

        ~IteratorAssignAdapter() = default;

        using IteratorStorage<I>::IteratorStorage;

        using IteratorStorage<I>::adapted;

        constexpr IteratorAssignAdapter &operator=(const IteratorAssignAdapter &other) noexcept {
            new (&adapted) I(other.adapted);
            return *this;
        }

        constexpr IteratorAssignAdapter &operator=(IteratorAssignAdapter &&other) noexcept {
            new (&adapted) I(std::move(other.adapted));
            return *this;
        }
    };

    template <typename I>
    using IteratorAssignLayer = std::conditional_t<std::movable<I>, IteratorStorage<I>, IteratorAssignAdapter<I>>;

    template <Iterator I, Sentinel<I> S>
    class AdapterIterator;

    template <Iterator I, Sentinel<I> S>
    class SentinelAdapter : private IteratorAssignLayer<S> {
        using Base = IteratorAssignLayer<S>;

      public:
        constexpr SentinelAdapter() = default;

        using Base::Base;

      private:
        using Base::adapted;

        friend class AdapterIterator<I, S>;
    };

    template <Iterator I, Sentinel<I> S>
    class AdapterIterator : private IteratorAssignLayer<I> {
        using Base = IteratorAssignLayer<I>;

      public:
        using iterator_category = std::input_iterator_tag;
        using value_type = std::remove_reference_t<DereferencedType<I>>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type *;
        using reference = value_type &;

        constexpr AdapterIterator() = default;

        using Base::Base;

        constexpr bool operator==(const SentinelAdapter<I, S> &sentinel) const {
            return !(adapted != sentinel.adapted);
        }

        constexpr decltype(auto) operator*() const {
            return *adapted;
        }

        constexpr AdapterIterator &operator++() {
            ++adapted;
            return *this;
        }

        void operator++(int) {
            ++adapted;
        }

      private:
        using Base::adapted;
    };
} // namespace retro::ranges