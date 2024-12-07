/**
 * @file Filter.h
 * @brief Specialized constexpr filter operation.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */

#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/FunctionTraits.h"
#include "RetroLib/Functional/BindFunctor.h"
#include <ranges>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif


namespace retro::ranges::views {

    /**
     * Filters a given range using a functor that determines which elements to include.
     *
     * This method applies a filtering operation on the provided range, producing a new
     * range containing only those elements for which the bound functor returns true.
     *
     * @param range A forwardable range of elements to filter. The range should support
     *              copying or moving and must be compatible with std::ranges::views::filter.
     * @return An instance of a range, resulting from the application of the filter view
     *         with the specified functor, containing only elements that satisfy the functor's criteria.
     */
    RETROLIB_EXPORT template <auto Functor, std::ranges::input_range R>
        requires HasFunctionCallOperator<decltype(Functor)>
    constexpr decltype(auto) filter(R&& range) {
        return std::ranges::views::filter(std::forward<R>(range), bind_functor<Functor>());
    }

    /**
     * Applies a filter operation on a range by utilizing a functor to determine
     * the criteria for selecting elements from the range. It uses the
     * standard library's ranges view filter mechanism.
     *
     * @return A range view that filters elements based on the provided functor.
     *         Only elements that satisfy the condition specified by the functor
     *         will be included in the view.
     */
    RETROLIB_EXPORT template <auto Functor>
        requires HasFunctionCallOperator<decltype(Functor)>
    constexpr decltype(auto) filter() {
        return std::ranges::views::filter(bind_functor<Functor>());
    }

}