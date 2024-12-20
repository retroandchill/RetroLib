/**
 * @file Transform.h
 * @brief Constexpr transform view adapter.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/Functional/CreateBinding.h"
#include "RetroLib/FunctionTraits.h"

#include <ranges>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro::ranges::views {

    /**
     * Applies a transformation to the elements in the given range by binding the provided arguments
     * to the transformation operation.
     *
     * @param range The input range to be transformed.
     * @param args The additional arguments to be used in conjunction with the transformation operation.
     * @return A new range where each element is a result of applying the transformation operation using
     *         the provided arguments.
     */
    RETROLIB_EXPORT template <std::ranges::input_range R, typename... A>
    constexpr decltype(auto) transform(R &&range, A &&...args) {
        return std::ranges::views::transform(std::forward<R>(range), create_binding(std::forward<A>(args)...));
    }

    /**
     * @brief Applies a transformation function to a range of elements.
     *
     * This method wraps the provided arguments to create a binding and applies
     * the std::ranges::views::transform view, which lazily transforms each
     * element in the range according to the specified transformation logic.
     *
     * @param args Variadic template arguments to be used for creating the transformation binding.
     *             These arguments are forwarded to the `create_binding` function.
     * @return A view that represents the transformed range of elements.
     */
    RETROLIB_EXPORT template <typename... A>
    constexpr decltype(auto) transform(A &&...args) {
        return std::ranges::views::transform(create_binding(std::forward<A>(args)...));
    }

    /**
     * @brief Transforms the given range using a specified functor.
     *
     * This function applies a transformation to each element of the provided range
     * by utilizing a functor. It uses the standard library's range transform view
     * to produce a new range where each element is the result of the functor applied
     * to the corresponding element in the input range.
     *
     * @tparam R A forwarding reference to the range type. Can be either an lvalue or rvalue.
     *           The range must satisfy the requirements imposed by std::ranges.
     * @tparam Functor The type of the functor used to transform the elements of the range.
     *                 The functor must be callable and return a type that can be stored
     *                 in the resulting range.
     *
     * @param range An R-value or L-value that represents the range of elements to be transformed.
     * @param args The binding arguments for the functor
     *
     * @return Returns a range that is the result of applying the transform operation using
     *         the specified functor on each element of the provided range.
     *
     * @note The function utilizes perfect forwarding for the input range to preserve
     *       its value category. The actual transformation is wrapped in a functor
     *       that is bound to the appropriate transformation mechanism.
     */
    RETROLIB_EXPORT template <auto Functor, std::ranges::input_range R, typename... A>
        requires HasFunctionCallOperator<decltype(Functor)>
    constexpr decltype(auto) transform(R &&range, A &&...args) {
        return std::ranges::views::transform(std::forward<R>(range), create_binding<Functor>(std::forward<A>(args)...));
    }

    /**
     * Applies a transformation to a range using a specified functor.
     *
     * This method leverages the C++ ranges library to transform each element
     * of a range by applying a functor. The functor is bound using the
     * `bind_functor` function, encapsulating the transformation logic.
     *
     * @param args The binding arguments for the functor
     * @return A view of the transformed range, where each element has been
     * modified according to the specified functor.
     */
    RETROLIB_EXPORT template <auto Functor, typename... A>
        requires HasFunctionCallOperator<decltype(Functor)>
    constexpr decltype(auto) transform(A &&...args) {
        return std::ranges::views::transform(create_binding<Functor>(std::forward<A>(args)...));
    }
} // namespace retro::ranges::views
