/**
 * @file Iterable.h
 * @brief Concepts pertaining to iterability.
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

namespace retro {
    template <typename T>
    concept Iterator = requires(T iter) {
        { ++iter } -> std::same_as<T &>;
        *iter;
    };

    template <typename S, typename I>
    concept Sentinel = requires(S s, I i) {
        { i != s } -> std::same_as<bool>;
    };

    template <typename T>
    using IteratorType = std::decay_t<decltype(std::declval<T>().begin())>;

    template <typename T>
    using SentinelType = std::decay_t<decltype(std::declval<T>().end())>;

    template <typename T>
    concept Iterable = requires(T &&range) {
        { std::forward<T>(range).begin() } -> Iterator;
        { std::forward<T>(range).end() } -> Sentinel<IteratorType<T>>;
    };
} // namespace retro