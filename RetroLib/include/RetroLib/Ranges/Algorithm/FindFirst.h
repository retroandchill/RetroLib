/**
 * @file FindFirst.h
 * @brief Finds the first value in a range when iterating over it.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "Retrolib/Functional/CreateBinding.h"
#include "Retrolib/Functional/ExtensionMethods.h"
#include "RetroLib/Optionals/Optional.h"
#include "Retrolib/Ranges/FeatureBridge.h"
#include "Retrolib/Utils/ForwardLike.h"

#include <ranges>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro::ranges {
    /**
     * Finds the first element in a given range and returns it wrapped in a specified output type.
     *
     * @param range The input range to search. Can be any range supporting begin and end operations.
     * @return An instance of the output type O containing the first element of the range,
     *         or a default-constructed instance of O if the range is empty.
     */
    RETROLIB_EXPORT template <optionals::OptionalType O, std::ranges::input_range R>
        requires std::constructible_from<O, RangeCommonReference<R>>
    constexpr O find_first(R &&range) {
        auto result = std::ranges::begin(range);
        return result != std::ranges::end(range) ? O(*forward_like<R>(result)) : O();
    }

    /**
     * Finds the first element in a range based on specific conditions and types.
     *
     * @tparam R The type of the range being passed to the function.
     * @param range The range object to inspect or process.
     * @return An object of type deduced from the range and conditions applied.
     */
    RETROLIB_EXPORT template <template <typename...> typename O = Optional, std::ranges::input_range R>
        requires optionals::OptionalType<O<std::ranges::range_value_t<R>>>
    constexpr auto find_first(R &&range) {
        if constexpr (std::is_lvalue_reference_v<RangeCommonReference<R>>) {
            if constexpr (optionals::RawReferenceOptionalValid<O, std::ranges::range_value_t<R>>) {
                return find_first<O<RangeCommonReference<R>>>(std::forward<R>(range));
            } else {
                return find_first<O<std::reference_wrapper<std::remove_reference_t<RangeCommonReference<R>>>>>(
                    std::forward<R>(range));
            }
        } else {
            return find_first<O<std::ranges::range_value_t<R>>>(std::forward<R>(range));
        }
    }

    /**
     * @struct FindFirstInvoker
     *
     * @brief A function object for finding the first occurrence of elements.
     *
     * This struct provides a call operator to find the first occurrence
     * of a specific element in a given range. The range must satisfy
     * the std::ranges::input_range concept, and the stored result type `O`
     * must be constructible from the common reference of the provided range.
     *
     * @tparam R The input range type that satisfies std::ranges::input_range.
     * @tparam O The output type that is constructible from the common reference
     *         of the provided range.
     *
     * @note This function object relies on the existence of a `find_first`
     * function, which must be defined elsewhere and take a range as an argument.
     * The returned value of `find_first<O>` should align with the expected semantics.
     *
     * @return O The result of invoking `find_first` on the given range.
     */
    template <optionals::OptionalType O>
    struct FindFirstInvoker {
        template <std::ranges::input_range R>
            requires std::constructible_from<O, RangeCommonReference<R>>
        constexpr O operator()(R &&range) const {
            return find_first<O>(std::forward<R>(range));
        }
    };

    /**
     * @brief A `constexpr` variable that provides an invoker for the "find first" operation.
     *
     * This variable is utilized to perform a specific operation tied to the type `O`,
     * facilitating its execution in a compile-time expression. It enables efficient
     * invocation of actions or algorithms that involve locating the first matching
     * element or occurrence based on the given context or logic.
     *
     * @tparam O The type parameter specifying the underlying context or operation
     * logic for which the invoker is defined.
     */
    template <optionals::OptionalType O>
    constexpr FindFirstInvoker<O> find_first_invoker;

    /**
     * @brief A callable structure that invokes the find_first function on a given range.
     *
     * This structure is used to apply the find_first function to a range that satisfies
     * specific requirements. It requires the range to be an input range, and the range's
     * value type must satisfy the requirements of an optional type.
     *
     * @tparam R The type of the input range which must satisfy std::ranges::input_range.
     *
     * @requires The value type of the range must be an optional type following the
     *           optionals::OptionalType<O<std::ranges::range_value_t<R>>> concept.
     *
     * @details The operator() function forwards the provided range to the find_first
     *          function, which performs the required operation. The structure is marked
     *          with constexpr, allowing evaluations at compile time if the provided
     *          arguments are compile-time constant.
     */
    template <template <typename...> typename O = Optional>
    struct FindFirstTemplateInvoker {
        template <std::ranges::input_range R>
            requires optionals::OptionalType<O<std::ranges::range_value_t<R>>>
        constexpr auto operator()(R &&range) const {
            return find_first<O>(std::forward<R>(range));
        }
    };

    /**
     * @brief A constant expression variable for invoking template-based functionality to find the first matching
     * element.
     *
     * This variable serves as a utility to facilitate operations that require locating the first element
     * in a collection or dataset that satisfies specific criteria, leveraging template-based customization.
     *
     * @tparam O A template parameter representing the type or configuration used in the invocation process.
     */
    template <template <typename...> typename O = Optional>
    constexpr FindFirstTemplateInvoker<O> find_first_template_invoker;

    /**
     * Finds the first occurrence of an element in a collection or sequence.
     *
     * This method leverages the 'find_first_invoker' to determine the first matching
     * element based on the specific criteria defined in the associated extension method.
     * It provides a consistent interface to retrieve the initial matching result.
     *
     * @return Returns a callable extension method constructed with 'find_first_invoker'
     *         for the specified type.
     */
    RETROLIB_EXPORT template <optionals::OptionalType O>
    constexpr auto find_first() {
        return extension_method<find_first_invoker<O>>();
    }

    /**
     * Finds and returns the first element in a collection that satisfies a given condition.
     *
     * This method integrates with an extension mechanism to provide the find-first functionality
     * using a template-based invoker.
     *
     * @return The result of the find-first operation as determined by the template invoker.
     */
    RETROLIB_EXPORT template <template <typename...> typename O = Optional>
    constexpr auto find_first() {
        return extension_method<find_first_template_invoker<O>>();
    }

} // namespace retro::ranges