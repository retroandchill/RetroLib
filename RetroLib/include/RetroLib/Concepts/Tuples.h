/**
 * @file Tuples.h
 * @brief Concepts pertaining to tuples.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "Retro/Utils/ForwardLike.h"

#include <tuple>
#include <type_traits>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro {
    RETROLIB_EXPORT template <typename T, size_t I>
    using ForwardedTupleType = std::conditional_t<std::is_lvalue_reference_v<std::tuple_element_t<I, std::decay_t<T>>>,
        std::tuple_element_t<I, std::decay_t<T>>,
        ForwardLikeType<T, std::tuple_element_t<I, std::decay_t<T>>>>;

    /**
     * Concept to check if a type is indexable by the given tuple element.
     *
     * @tparam T The candidate tuple type
     * @tparam I The index to try and get
     */
    RETROLIB_EXPORT template <typename T, size_t I>
    concept HasTupleElement = requires(T t) {
        { get<I>(t) } -> std::convertible_to<std::tuple_element_t<I, T>&>;
    } || requires(T t) {
        { std::get<I>(t) } -> std::convertible_to<std::tuple_element_t<I, T>&>;
    };

    /**
     * Evaluates at compile-time if all specified indices in a given index sequence
     * have an associated tuple element defined for the type `T`.
     *
     * @param seq A compile-time index sequence represented by std::index_sequence<I...>.
     *            It contains a sequence of indices that will be checked to ensure each
     *            corresponds to a valid tuple element in `T`.
     *
     * @return A compile-time boolean value indicating whether all indices in the index
     *         sequence correspond to valid tuple elements within the type `T`. Returns true
     *         if all indices*/
    RETROLIB_EXPORT template <typename T, size_t... I>
    consteval bool all_has_tuple_element(std::index_sequence<I...>) {
        return (HasTupleElement<T, I> && ...);
    }

    /**
     * Concept to verify if a type is like a tuple or not (and hence can be used with structured bindings and std::apply).
     *
     * @tparam T The type to check the tuple status against
     */
    RETROLIB_EXPORT template <typename T>
    concept TupleLike = requires {
        typename std::tuple_size<T>::type;
        { std::tuple_size_v<T> } -> std::convertible_to<size_t>;
    } && all_has_tuple_element<T>(std::make_index_sequence<std::tuple_size_v<T>>{});

    template <typename F, typename T, size_t... I>
    concept CanApplyArgs = HasFunctionCallOperator<std::decay_t<F>> && TupleLike<std::decay_t<T>> && std::invocable<F, ForwardedTupleType<T, I>...>;

    template <typename F, typename T, size_t... I>
    consteval bool can_apply_index_sequence(std::index_sequence<I...>) {
        return CanApplyArgs<F, T, I...>;
    }

    RETROLIB_EXPORT template <typename F, typename T>
    concept CanApply = HasFunctionCallOperator<std::decay_t<F>> && TupleLike<std::decay_t<T>> && can_apply_index_sequence<F, T>(std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>{});

    template <typename F, typename T, size_t... I>
    concept NoThrowApplicableArgs = HasFunctionCallOperator<std::decay_t<F>> && TupleLike<std::decay_t<T>> && std::is_nothrow_invocable_v<F, ForwardedTupleType<T, I>...>;

    template <typename F, typename T, size_t... I>
    consteval bool is_nothrow_applicable(std::index_sequence<I...>) {
        return NoThrowApplicableArgs<F, T, I...>;
    }

    RETROLIB_EXPORT template <typename F, typename T>
    concept NoThrowApplicable = HasFunctionCallOperator<std::decay_t<F>> && TupleLike<std::decay_t<T>> && is_nothrow_applicable<F, T>(std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>{});
}