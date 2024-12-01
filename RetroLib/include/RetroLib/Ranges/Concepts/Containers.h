/**
 * @file Containers.h
 * @brief Concept definitions regarding containers
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include <ranges>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro::ranges {

    template <typename T>
    concept StlReservable = std::ranges::sized_range<T> && requires(T &container, std::ranges::range_size_t<T> size) {
        container.reserve(size);
    };

    template <typename T>
    concept StlCapacity = requires(T &container) {
        container.capacity();
    };

    template <typename T>
    concept StlMaxSize = requires(T &container) {
        container.max_size();
    };

    RETROLIB_EXPORT template <StlReservable T>
    constexpr void container_reserve(T& range, std::ranges::range_size_t<T> size) {
        range.reserve(size);
    }

    RETROLIB_EXPORT template <StlCapacity T>
    constexpr auto container_capacity(const T& range) {
        return range.capacity();
    }

    RETROLIB_EXPORT template <StlMaxSize T>
    constexpr auto container_max_size(const T& range) {
        return range.max_size();
    }

    RETROLIB_EXPORT template <typename T>
    concept ReservableContainer = std::ranges::sized_range<T> &&
        requires(T &container, std::ranges::range_size_t<T> size) {
            container_reserve(container, size);
            { container_capacity(container) } -> std::convertible_to<std::ranges::range_size_t<T>>;
            { container_max_size(container) } -> std::convertible_to<std::ranges::range_size_t<T>>;
        };

    template< typename C, typename R >
    concept StlEmplaceBack = requires (C& c, R&& ref) {
        c.emplace_back(std::forward<R>(ref));
    };

    template< typename C, typename R >
    concept StlPushBack = requires (C& c, R&& ref) {
        c.push_back(std::forward<R>(ref));
    };

    template< typename C, typename R >
    concept StlEmplace = requires (C& c, R&& ref) {
        c.emplace(std::forward<R>(ref));
    };

    template< typename C, typename R >
    concept StlInsert = requires (C& c, R&& ref) {
        c.insert(std::forward<R>(ref));
    };

    template< typename Container, typename Reference >
    concept StlAppendable = StlEmplaceBack<Container, Reference> || StlPushBack<Container, Reference> || StlEmplace<Container, Reference> || StlInsert<Container, Reference>;

    template <typename C, typename T>
        requires StlAppendable<C, T>
    constexpr auto append_container(C& container, T&& value) {
        if constexpr (StlEmplaceBack<C, T>) {
            return container.emplace_back(std::forward<T>(value));
        } else if constexpr (StlPushBack<C, T>) {
            return container.push_back(std::forward<T>(value));
        } else if constexpr (StlEmplace<C, T>) {
            return container.emplace(std::forward<T>(value));
        } else if constexpr (StlInsert<C, T>) {
            return container.insert(std::forward<T>(value));
        }
    }

    template <typename C, typename T>
    concept AppendableContainer = requires (C& container, T&& value) {
        append_container(container, std::forward<T>(value));
    };

    template <typename R, typename T>
    concept ContainerCompatibleRange = std::ranges::input_range<R> && std::convertible_to<std::ranges::range_common_reference_t<R>, T>;
}