/**
 * @file Reduce.h
 * @brief Functional chain component used for reducing a range down to a single value.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/Functional/CreateBinding.h"
#include "RetroLib/Functional/ExtensionMethods.h"
#include "RetroLib/Ranges/FeatureBridge.h"
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace Retro::Ranges {
    struct ReduceInvoker {
        template <std::ranges::input_range R, typename I, HasFunctionCallOperator F>
            requires std::invocable<F, I, RangeCommonReference<R>> &&
                 std::convertible_to<std::invoke_result_t<F, I, RangeCommonReference<R>>, I>
        constexpr auto operator()(R &&Range, I &&Identity, F Functor) {
            auto Result = std::forward<I>(Identity);
            for (auto &&Value : std::forward<R>(Range)) {
                Result = std::invoke(Functor, std::move(Result), std::forward<decltype(Value)>(Value));
            }
            return Result;
        }
    };

    constexpr ReduceInvoker ReduceFunction;

    template <auto Functor = DynamicFunctor>
      requires ValidFunctorParameter<Functor>
  constexpr FunctorBindingInvoker<Functor, ReduceFunction> ReduceCallback;

    RETROLIB_EXPORT template <auto Functor = DynamicFunctor, typename... A>
        requires ValidFunctorParameter<Functor>
    constexpr auto Reduce(A &&...Args) {
        return ExtensionMethod<ReduceCallback<Functor>>(std::forward<A>(Args)...);
    }
} // namespace retro::ranges