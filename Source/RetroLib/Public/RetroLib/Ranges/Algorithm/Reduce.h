/**
 * @file Reduce.h
 * @brief Functional chain component used for reducing a range down to a single value.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#include "RetroLib/Functional/CreateBinding.h"
#include "RetroLib/Functional/ExtensionMethods.h"
#include "RetroLib/Ranges/FeatureBridge.h"

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace Retro::Ranges {
    /**
     * @brief Reduces a range of elements into a single value by applying a functor
     * iteratively.
     *
     * This function takes a range, an initial identity value, and a functor
     * to combine the elements in the range into a single aggregated result.
     * The functor is applied in left-to-right order.
     *
     * @tparam R The type of the range.
     * @tparam I The type of the identity value.
     * @tparam F The type of the functor.
     * @param Range The range of elements to be reduced.
     * @param Identity The initial identity value used in the reduction process.
     * @param Functor A callable object that specifies how to combine two elements.
     * @return The final aggregated result of the reduction operation.
     */
    RETROLIB_EXPORT template <std::ranges::input_range R, typename I, HasFunctionCallOperator F>
        requires std::invocable<F, I, TRangeCommonReference<R>> &&
                 std::convertible_to<std::invoke_result_t<F, I, TRangeCommonReference<R>>, I>
    constexpr auto Reduce(R &&Range, I &&Identity, F Functor) {
        auto Result = std::forward<I>(Identity);
        for (auto &&Value : std::forward<R>(Range)) {
            Result = std::invoke(Functor, std::move(Result), std::forward<decltype(Value)>(Value));
        }
        return Result;
    }

    /**
     * Reduces a given range by performing a folding operation using the specified identity value
     * and a binding created from the provided arguments.
     *
     * @param Range The range of elements to be reduced.
     * @param Identity The identity value used for the reduction operation.
     * @param PrimeArg The primary argument to create a binding for the reduction operation.
     * @param Args Additional arguments to create a binding for the reduction.
     * @return The result of folding the elements in the range using the specified identity value
     * and the created binding.
     */
    RETROLIB_EXPORT template <std::ranges::input_range R, typename I, typename T, typename... A>
        requires std::invocable<TBindingType<T, A...>, I, TRangeCommonReference<R>> &&
                 std::convertible_to<std::invoke_result_t<TBindingType<T, A...>, I, TRangeCommonReference<R>>, I>
    constexpr auto Reduce(R &&Range, I &&Identity, T &&PrimeArg, A &&...Args) {
        return Reduce(std::forward<R>(Range), std::forward<I>(Identity),
                      CreateBinding(std::forward<T>(PrimeArg), std::forward<A>(Args)...));
    }

    /**
     * Reduces the provided range using the specified identity and arguments to create a binding
     * using the Functor. This method enables the application of a functional object
     * to the range with a defined starting value.
     *
     * @tparam R The type of the range to reduce.
     * @tparam I The type of the identity value.
     * @tparam A The types of additional arguments required for creating a binding.
     * @param Range The range of elements to process with reduction.
     * @param Identity The initial value for the reduction operation.
     * @param Args Additional arguments for creating the binding for the Functor.
     * @return The result of reducing the range using the identity value and the created binding.
     */
    RETROLIB_EXPORT template <auto Functor, std::ranges::input_range R, typename I, typename... A>
        requires HasFunctionCallOperator<decltype(Functor)> &&
                 std::invocable<TConstBindingType<Functor, A...>, I, TRangeCommonReference<R>> &&
                 std::convertible_to<std::invoke_result_t<TConstBindingType<Functor, A...>, I, TRangeCommonReference<R>>,
                                     I>
    constexpr auto Reduce(R &&Range, I &&Identity, A &&...Args) {
        return Reduce(std::forward<R>(Range), std::forward<I>(Identity),
                      CreateBinding<Functor>(std::forward<A>(Args)...));
    }

    /**
     * @struct FReduceInvoker
     *
     * @brief A functor for invoking the `reduce` operation on a given input range.
     *
     * The `ReduceInvoker` struct provides an overloaded call operator that allows invoking
     * the `reduce` algorithm on a provided range, identity element, and additional arguments.
     * The `reduce` operation applies a user-defined operation for combining elements of the range
     * with an identity value and optional parameters.
     *
     * @tparam R A type representing an input range that satisfies `std::ranges::input_range`.
     * @tparam I The type of the identity element.
     * @tparam A Additional argument types forwarded to the `reduce` operation.
     *
     * @note The `reduce` operation assumes the presence of a callable object that can be invoked
     * with the provided identity value and range's common reference. The callable's result must be
     * convertible to the same type as the identity value.
     *
     * @remarks The `operator()` requires that:
     *   - The provided invocable conforms to `std::invocable<BindingType<A...>, I, RangeCommonReference<R>>`.
     *   - The result of the invocable is convertible to the type of the identity value.
     */
    struct FReduceInvoker {
        /**
         * @brief Invokes the `reduce` operation on a given range with an identity value and additional arguments.
         *
         * This function call operator applies the `reduce` algorithm to the specified range using
         * the provided identity element and forwarded additional arguments.
         *
         * @tparam R The type representing the input range, which must satisfy `std::ranges::input_range`.
         * @tparam I The type of the identity element used in the `reduce` operation.
         * @tparam A Variadic template for additional argument types forwarded to the `reduce` function.
         *
         * @param Range The input range over which the `reduce` operation will be applied.
         * @param Identity The identity element used for the `reduce` algorithm.
         * @param Args Additional arguments forwarded to the `reduce` operation.
         *
         * @return The result of the `reduce` operation, which combines the elements of the input range
         * starting with the identity element, using the specified operation logic.
         */
        template <std::ranges::input_range R, typename I, typename... A>
            requires std::invocable<TBindingType<A...>, I, TRangeCommonReference<R>> &&
                     std::convertible_to<std::invoke_result_t<TBindingType<A...>, I, TRangeCommonReference<R>>, I>
        constexpr auto operator()(R &&Range, I &&Identity, A &&...Args) const {
            return Reduce(std::forward<R>(Range), std::forward<I>(Identity), std::forward<A>(Args)...);
        }
    };

    /**
     * @var reduce_invoker
     *
     * @brief A constant instance of the `ReduceInvoker` functor for performing `reduce` operations.
     *
     * The `reduce_invoker` is a `constexpr` object that provides access to the `reduce` operation
     * through its call operator. It allows the reduction over input ranges using a specified
     * identity element and a user-defined combining operation.
     *
     * @details This instance facilitates the use of the `ReduceInvoker` without requiring
     * explicit instantiation. It relies on the templated `ReduceInvoker` struct to perform
     * reduction computations in a constexpr context.
     *
     * @note The availability and usage of `reduce_invoker` depend on the constraints and requirements
     * defined by the `ReduceInvoker` struct, including the necessary operator overloading and type compatibility.
     */
    constexpr FReduceInvoker ReduceFuction;

    /**
     * @struct TReduceConstInvoker
     *
     * @brief A constant functor for invoking the `reduce` operation on a given input range.
     *
     * The `ReduceConstInvoker` struct provides an overloaded call operator that enables invoking
     * the `reduce` algorithm on a specified input range, identity value, and additional arguments,
     * using a constant invocation context.
     *
     * The `reduce` operation combines elements of an input range with an identity value, applying a
     * user-defined operation that is invoked through a constant callable object.
     *
     * @tparam R A type representing an input range that satisfies `std::ranges::input_range`.
     * @tparam I The type of the identity element.
     * @tparam A Types of additional arguments forwarded to the `reduce` operation.
     *
     * @note The `reduce` operation requires a callable object capable of being invoked with the
     * provided identity value and the common reference type of the input range. The callable's
     * result must be convertible to the type of the identity value.
     *
     * @remarks The `operator()` imposes the following requirements:
     *   - The provided invocable must be compatible with `std::invocable<ConstBindingType<Functor, A...>, I,
     * RangeCommonReference<R>>`.
     *   - The result of invocation must be convertible to the type of the identity value.
     */
    template <auto Functor>
        requires HasFunctionCallOperator<decltype(Functor)>
    struct TReduceConstInvoker {
        /**
         * @brief Invokes the `reduce` operation on a provided input range, identity element, and additional arguments.
         *
         * This operator applies the `reduce` algorithm using a specified functor on the given range. It combines
         * elements of the range with an identity value and forwards any additional arguments required for the
         * operation.
         *
         * @tparam R The type of the input range; must satisfy `std::ranges::input_range`.
         * @tparam I The type of the identity element.
         * @tparam A Variadic types for additional arguments forwarded to the `reduce` operation.
         *
         * @param Range The input range on which the `reduce` operation is applied.
         * @param Identity The identity element used as the initial value for the `reduce` operation.
         * @param Args Additional arguments forwarded to the provided functor during the reduction.
         *
         * @return The result of the `reduce` operation, combining the elements of the range with the identity value,
         * using the specified functor. The result must be convertible to the type of the identity value.
         */
        template <std::ranges::input_range R, typename I, typename... A>
            requires std::invocable<TConstBindingType<Functor, A...>, I, TRangeCommonReference<R>> &&
                     std::convertible_to<
                         std::invoke_result_t<TConstBindingType<Functor, A...>, I, TRangeCommonReference<R>>, I>
        constexpr auto operator()(R &&Range, I &&Identity, A &&...Args) const {
            return Reduce<Functor>(std::forward<R>(Range), std::forward<I>(Identity), std::forward<A>(Args)...);
        }
    };

    /**
     * @var reduce_const_invoker
     *
     * @brief A constant invoker for the `reduce` operation with a predefined callable.
     *
     * The `reduce_const_invoker` is a `constexpr` instance that provides functionality
     * to perform a `reduce` operation using a predefined functor. It allows applying a
     * reduction operation on a range with a specified identity element and any additional
     * parameters, leveraging the callable object defined as its template parameter.
     *
     * @tparam Functor A callable type that specifies the reduction operation. The functor
     * must be invocable with the identity value and elements of the provided range, producing
     * results convertible to the identity's type.
     *
     * @note The `reduce_const_invoker` is designed to be a stateless, compile-time constant
     * for efficient use in various scenarios involving reduction operations.
     *
     * @remarks Usage requires:
     *   - The callable `Functor` to conform to the invocable concept with the parameters:
     *     identity value, element type of the range, and optionally other arguments.
     *   - The callable’s result to be assignable to the type of the identity element.
     */
    template <auto Functor>
        requires HasFunctionCallOperator<decltype(Functor)>
    constexpr TReduceConstInvoker<Functor> ReduceConstFunction;

    /**
     * @fn reduce
     *
     * @brief Performs a reduction operation on the provided identity value and arguments.
     *
     * The `reduce` function applies the `reduce_invoker` algorithm to the given identity value
     * and additional arguments to compute a result. The function is a wrapper for invoking the
     * `reduce_invoker` implementation via an extension method.
     *
     * @tparam I The type of the identity element.
     * @tparam A Variadic template representing additional arguments passed to the operation.
     *
     * @param Identity The identity element used as the starting point for the reduction operation.
     * @param Args Additional arguments forwarded to the `reduce_invoker` implementation.
     *
     * @return The result of invoking the `reduce_invoker` operation with the provided arguments.
     *
     * @note This function relies on the `ExtensionMethod` mechanism for leveraging the `reduce_invoker`.
     */
    RETROLIB_EXPORT template <typename I, typename... A>
    constexpr auto Reduce(I &&Identity, A &&...Args) {
        return ExtensionMethod<ReduceFuction>(std::forward<I>(Identity), std::forward<A>(Args)...);
    }

    /**
     * @brief Performs a reduction operation over a range of elements with a specified identity value and additional
     * arguments.
     *
     * The `reduce` function applies a reduction operation using a provided identity value and any additional arguments.
     * The operation is implemented by invoking an extension method with a specialized invoker.
     *
     * @tparam I The type of the identity value.
     * @tparam A The types of additional arguments forwarded to the reduction operation.
     *
     * @param Identity The initial value for the reduction operation, which serves as the identity element.
     * @param Args Additional arguments required by the reduction operation, forwarded appropriately.
     *
     * @return The result of the reduction operation performed on the provided range and arguments.
     *
     * @note This function assumes the existence of a callable (`reduce_const_invoker<Functor>`)
     * to perform the reduction logic. The types of the identity value and additional arguments
     * must align with the requirements of the callable object used in the implementation.
     */
    RETROLIB_EXPORT template <auto Functor, typename I, typename... A>
        requires HasFunctionCallOperator<decltype(Functor)>
    constexpr auto Reduce(I &&Identity, A &&...Args) {
        return ExtensionMethod<ReduceConstFunction<Functor>>(std::forward<I>(Identity), std::forward<A>(Args)...);
    }
} // namespace retro::ranges