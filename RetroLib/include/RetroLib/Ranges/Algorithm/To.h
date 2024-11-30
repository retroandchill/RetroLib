/**
 * @file To.h
 * @brief Creates the view extension to convert it into a collection type.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "Ranges/Concepts/Container.h"

#include <ranges>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro::ranges {
    RETROLIB_EXPORT template <typename C, std::ranges::input_range R, typename... A>
        requires (!std::ranges::view<C>)
    constexpr C to(R&& range, A&&... args) {
        C result(std::forward<A>(args)...);

        if constexpr (std::ranges::sized_range<R> && ReservableContainer<C>) {
            container_reserve(result, std::ranges::size(range));
        }

        using RangeType = std::ranges::range_common_reference_t<R>;
        for (RangeType x : range) {
            append_container(result, std::forward<RangeType>(x));
        }

        return result;
    }

    RETROLIB_EXPORT template <template<typename...> typename C, std::ranges::input_range R, typename... A>
    constexpr auto to(R&& range, A&&... args) {
        using RangeType = std::ranges::range_value_t<R>;
        return to<C<RangeType>>(std::forward<R>(range), std::forward<A>(args)...);
    }

    template <typename C>
        requires (!std::ranges::view<C>)
    struct ToInvoker {
        template <std::ranges::input_range R, typename... A>
        constexpr C operator()(R&& range, A&&... args) const {
            return to<C>(std::forward<R>(range), std::forward<A>(args)...);
        }
    };

    template <typename C>
        requires (!std::ranges::view<C>)
    constexpr ToInvoker<C> to_invoker;

    template <template<typename...> typename C>
    struct TemplatedToInvoker {
        template <std::ranges::input_range R, typename... A>
        constexpr auto operator()(R&& range, A&&... args) const {
            return to<C>(std::forward<R>(range), std::forward<A>(args)...);
        }
    };

    RETROLIB_EXPORT template <template<typename...> typename C>
    constexpr TemplatedToInvoker<C> template_to_invoker;


    RETROLIB_EXPORT template <typename C, typename... A>
        requires (!std::ranges::view<C>)
    constexpr auto to(A&&... args) {
        return extension_method<to_invoker<C>>(std::forward<A>(args)...);
    }

    RETROLIB_EXPORT template <template<typename...> typename C, typename... A>
    constexpr auto to(A&&... args) {
        return extension_method<template_to_invoker<C>>(std::forward<A>(args)...);
    }


}
