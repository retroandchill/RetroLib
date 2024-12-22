/**
 * @file FunctionalClosure.h
 * @brief A specialized version of an extension method that handles the binding of functors.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */

#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/Functional/CreateBinding.h"
#include "RetroLib/Functional/ExtensionMethod.h"
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro {

    template <auto BoundFunctor, auto BaseFunctor>
        requires (ValidFunctorParameter<BoundFunctor> && is_valid_functor_object(BaseFunctor))
    struct FunctorBindingInvoker {
        using BoundFunctorType = decltype(BoundFunctor);
        using BaseFunctorType = decltype(BaseFunctor);

        template <typename T, typename F>
            requires std::invocable<BaseFunctorType, T, F> && DynamicFunctorBinding<BoundFunctor>
        constexpr auto operator()(T&& operand, F&& functor) const {
            return std::invoke(BaseFunctor, std::forward<T>(operand), std::forward<F>(functor));
        }

        template <typename T, typename... A>
            requires (sizeof...(A) > 1) && std::invocable<BaseFunctorType, T, BindingType<A...>> && DynamicFunctorBinding<BoundFunctor>
        constexpr auto operator()(T&& operand, A&&... args) const {
            return std::invoke(BaseFunctor, std::forward<T>(operand), create_binding(std::forward<A>(args)...));
        }

        template <typename T>
            requires std::invocable<BaseFunctorType, T, BoundFunctorType> && (!DynamicFunctorBinding<BoundFunctor>)
        constexpr auto operator()(T&& operand) const {
            return std::invoke(BaseFunctor, std::forward<T>(operand), create_binding<BoundFunctor>());
        }

        template <typename T, typename... A>
            requires (sizeof...(A) >= 1) && std::invocable<BaseFunctorType, T, BindingType<BoundFunctorType, A...>> && (!DynamicFunctorBinding<BoundFunctor>)
        constexpr auto operator()(T&& operand, A&&... args) const {
            return std::invoke(BaseFunctor, std::forward<T>(operand), create_binding<BoundFunctor>(std::forward<A>(args)...));
        }
    };

}