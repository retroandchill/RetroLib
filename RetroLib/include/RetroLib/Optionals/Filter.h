/**
 * @file Filter.h
 * @brief Closure for filtering an optional value
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/Optionals/OptionalOperations.h"
#include "RetroLib/Functional/Invoke.h"
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro::optionals {

    /**
     * Filters the given optional value using the specified functor.
     *
     * This function applies a functor to the value contained within an optional.
     * If the optional has a value and the functor returns true, it returns the original
     * optional. Otherwise, it returns an empty optional.
     *
     * If the optional is a reference, a reference is returned, otherwise a value is returned.
     *
     * @tparam O The optional object that was passed along
     * @tparam F The functor to operate on
     * @param optional The optional value to be filtered. Can be an lvalue or rvalue reference.
     * @param functor A callable object that takes the value from the optional and returns a boolean.
     * @return The original optional if it has a value and the functor returns true, otherwise an empty optional.
     */
    RETROLIB_EXPORT template <Optional O, typename F>
        requires std::is_invocable_r_v<bool, F, CommonReference<O>>
    constexpr auto filter(O&& optional, F&& functor) {
        if constexpr (std::is_lvalue_reference_v<O>) {
            using FilteredType = decltype(make_optional_reference(std::forward<O>(optional)));
            return has_value(std::forward<O>(optional)) && std::invoke(std::forward<F>(functor), get<O>(std::forward<O>(optional))) ? make_optional_reference(std::forward<O>(optional)) : FilteredType();
        } else {
            return has_value(std::forward<O>(optional)) && std::invoke(std::forward<F>(functor), get<O>(std::forward<O>(optional))) ? O(std::forward<O>(optional)) : O();
        }
    }

    /**
     * Filters the given optional based on a functor condition.
     *
     * This function checks if the optional has a value and if the functor condition is satisfied.
     * If both are true, it returns the original optional; otherwise, it returns an empty optional.
     * This is performed differently for lvalue and rvalue references.
     *
     * @tparam O The type of the optional being filtered.
     * @param optional The optional value to filter. This can be an lvalue or rvalue reference.
     * @return A filtered optional, which contains its original value if the condition is satisfied, or is empty otherwise.
     */
    RETROLIB_EXPORT template <auto Functor, Optional O>
        requires std::is_invocable_r_v<bool, decltype(Functor), CommonReference<O>>
    constexpr auto filter(O&& optional) {
        if constexpr (std::is_lvalue_reference_v<O>) {
            using FilteredType = decltype(make_optional_reference(std::forward<O>(optional)));
            return has_value(std::forward<O>(optional)) && invoke<Functor>(get(std::forward<O>(optional))) ? make_optional_reference(std::forward<O>(optional)) : FilteredType();
        } else {
            return has_value(std::forward<O>(optional)) && invoke<Functor>(get(std::forward<O>(optional))) ? O(std::forward<O>(optional)) : O();
        }
    }

    struct FilterInvoker {
        template <Optional O, typename F>
            requires std::is_invocable_r_v<bool, F, CommonReference<O>>
        constexpr auto operator()(O&& optional, F&& functor) const {
            return filter(std::forward<O>(optional), std::forward<F>(functor));
        }
    };

    constexpr FilterInvoker filter_invoker;

    template <auto Functor>
    struct ConstFilterInvoker {
        template <Optional O>
            requires std::is_invocable_r_v<bool, decltype(Functor), CommonReference<O>>
        constexpr auto operator()(O&& optional) const {
            return filter<Functor>(std::forward<O>(optional));
        }
    };

    template <auto Functor>
    constexpr ConstFilterInvoker<Functor> const_filter_invoker;

    RETROLIB_EXPORT template <typename F>
    constexpr auto filter(F&& functor) {
        return extension_method<filter_invoker>(std::forward<F>(functor));
    }

    RETROLIB_EXPORT template <auto Functor>
    constexpr auto filter() {
        return extension_method<const_filter_invoker<Functor>>();
    }

}