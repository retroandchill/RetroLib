/**
 * @file BindFunctor.h
 * @brief TODO: Fill me out
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/Concepts/ParameterPacks.h"
#include "RetroLib/FunctionTraits.h"
#include "RetroLib/Utils/ForwardLike.h"

#include <tuple>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro {

    template <auto Functor, typename... A>
        requires HasFunctionCallOperator<decltype(Functor)> || Member<decltype(Functor)>
    struct ConstBindingArgumentStorage {
        using F = decltype(Functor);
        using ArgsTuple = std::tuple<A...>;

        template <typename... T>
            requires std::constructible_from<ArgsTuple, T...> && (!PackSameAs<ConstBindingArgumentStorage, T...>)
        constexpr explicit ConstBindingArgumentStorage(T &&...args) : args(std::forward<T>(args)...) {
        }

        template <typename... T>
            requires std::invocable<F, T..., A&...>
        constexpr decltype(auto) operator()(T&&... call_args) & noexcept(std::is_nothrow_invocable_v<F, T..., A&...>) {
            return std::apply(
                [&]<typename... U>(U &&...final_args) -> decltype(auto) {
                    return std::invoke(Functor, std::forward<T>(call_args)..., std::forward<U>(final_args)...);
                },
                args);
        }

        template <typename... T>
            requires std::invocable<F, T..., const A&...>
        constexpr decltype(auto) operator()(T&&... call_args) & noexcept(std::is_nothrow_invocable_v<F, T..., const A&...>) {
            return std::apply(
                [&]<typename... U>(U &&...final_args) -> decltype(auto) {
                    return std::invoke(Functor, std::forward<T>(call_args)..., std::forward<U>(final_args)...);
                },
                args);
        }

        template <typename... T>
            requires std::invocable<F, T..., A...>
        constexpr decltype(auto) operator()(T&&... call_args) && noexcept(std::is_nothrow_invocable_v<F, T..., A...>) {
            return std::apply(
                [&]<typename... U>(U &&...final_args) -> decltype(auto) {
                    return std::invoke(Functor, std::forward<T>(call_args)..., std::forward<U>(final_args)...);
                },
                std::move(args));
        }

    private:
        ArgsTuple args;
    };

    template <auto Functor, typename C, typename... A>
        requires Member<decltype(Functor)> && std::is_member_function_pointer_v<C>
    struct ConstBindingArgumentStorage<Functor, C, A...> {
        using F = decltype(Functor);
        using ArgsTuple = std::tuple<A...>;

        template <typename S, typename... T>
            requires std::convertible_to<S, C> && std::constructible_from<ArgsTuple, T...> && (!std::same_as<std::decay_t<S>, ConstBindingArgumentStorage>)
        constexpr explicit ConstBindingArgumentStorage(C&& obj, T &&...args) : obj(std::forward<C>(obj)), args(std::forward<T>(args)...) {
        }

        template <typename... T>
            requires std::invocable<F, T..., C&, A&...>
        constexpr decltype(auto) operator()(T&&... call_args) & noexcept(std::is_nothrow_invocable_v<F, T..., C&, A&...>) {
            return std::apply(
                [&]<typename... U>(U &&...final_args) -> decltype(auto) {
                    return std::invoke(Functor, std::forward<T>(call_args)..., obj, std::forward<U>(final_args)...);
                },
                args);
        }

        template <typename... T>
            requires std::invocable<F, T..., const C&, const A&...>
        constexpr decltype(auto) operator()(T&&... call_args) & noexcept(std::is_nothrow_invocable_v<F, T..., const C&, const A&...>) {
            return std::apply(
                [&]<typename... U>(U &&...final_args) -> decltype(auto) {
                    return std::invoke(Functor, std::forward<T>(call_args)..., obj, std::forward<U>(final_args)...);
                },
                args);
        }

        template <typename... T>
            requires std::invocable<F, T..., C, A...>
        constexpr decltype(auto) operator()(T&&... call_args) && noexcept(std::is_nothrow_invocable_v<F, T..., C, A...>) {
            return std::apply(
                [&]<typename... U>(U &&...final_args) -> decltype(auto) {
                    return std::invoke(Functor, std::forward<T>(call_args)..., obj, std::forward<U>(final_args)...);
                },
                std::move(args));
        }

        template <typename... T>
            requires std::invocable<F, C&, T..., A&...>
        constexpr decltype(auto) operator()(T&&... call_args) & noexcept(std::is_nothrow_invocable_v<F, C&, T..., A&...>) {
            return std::apply(
                [&]<typename... U>(U &&...final_args) -> decltype(auto) {
                    return std::invoke(Functor, obj, std::forward<T>(call_args)..., std::forward<U>(final_args)...);
                },
                args);
        }

        template <typename... T>
            requires std::invocable<F, const C&, T..., const A&...>
        constexpr decltype(auto) operator()(T&&... call_args) & noexcept(std::is_nothrow_invocable_v<F, const C&, T..., const A&...>) {
            return std::apply(
                [&]<typename... U>(U &&...final_args) -> decltype(auto) {
                    return std::invoke(Functor, obj, std::forward<T>(call_args)..., std::forward<U>(final_args)...);
                },
                args);
        }

        template <typename... T>
            requires std::invocable<F, C, T..., A...>
        constexpr decltype(auto) operator()(T&&... call_args) && noexcept(std::is_nothrow_invocable_v<F, C, T..., A...>) {
            return std::apply(
                [&]<typename... U>(U &&...final_args) -> decltype(auto) {
                    return std::invoke(Functor, obj, std::forward<T>(call_args)..., std::forward<U>(final_args)...);
                },
                std::move(args));
        }

    private:
        C obj;
        ArgsTuple args;
    };

    template <auto Functor>
        requires HasFunctionCallOperator<decltype(Functor)> || Member<decltype(Functor)>
    struct ConstBindingArgumentStorage<Functor> {
        using F = decltype(Functor);

        ConstBindingArgumentStorage() = default;

        template <typename... T>
            requires std::invocable<F, T...>
        constexpr decltype(auto) operator()(T&&... args) const noexcept(std::is_nothrow_invocable_v<F, T...>) {
            return std::invoke(Functor, std::forward<T>(args)...);
        }

    };

    template <auto Functor, typename A>
        requires HasFunctionCallOperator<decltype(Functor)> || Member<decltype(Functor)>
    struct ConstBindingArgumentStorage<Functor, A> {
        using F = decltype(Functor);

        template <typename T>
            requires std::convertible_to<T, A> && (!std::same_as<std::decay_t<T>, ConstBindingArgumentStorage>)
        constexpr explicit ConstBindingArgumentStorage(T &&arg) : arg(std::forward<T>(arg)) {
        }

        template <typename... T>
            requires std::invocable<F, T..., A&>
        constexpr decltype(auto) operator()(T&&... call_args) & noexcept(std::is_nothrow_invocable_v<F, T..., A&>) {
            return std::invoke(Functor, std::forward<T>(call_args)..., arg);
        }

        template <typename... T>
            requires std::invocable<F, T..., const A&>
        constexpr decltype(auto) operator()(T&&... call_args) & noexcept(std::is_nothrow_invocable_v<F, T..., const A&>) {
            return std::invoke(Functor, std::forward<T>(call_args)..., arg);
        }

        template <typename... T>
            requires std::invocable<F, T..., A>
        constexpr decltype(auto) operator()(T&&... call_args) && noexcept(std::is_nothrow_invocable_v<F, T..., A>) {
            return std::invoke(Functor, std::forward<T>(call_args)..., std::move(arg));
        }

        template <typename... T>
            requires std::invocable<F, A&, T...> && std::is_member_pointer_v<F>
        constexpr decltype(auto) operator()(T&&... call_args) & noexcept(std::is_nothrow_invocable_v<F, A&, T...>) {
            return std::invoke(Functor, arg, std::forward<T>(call_args)...);
        }

        template <typename... T>
            requires std::invocable<F, const A&, T...> && std::is_member_pointer_v<F>
        constexpr decltype(auto) operator()(T&&... call_args) & noexcept(std::is_nothrow_invocable_v<F, const A&, T...>) {
            return std::invoke(Functor, arg, std::forward<T>(call_args)...);
        }

        template <typename... T>
            requires std::invocable<F, A, T...> && std::is_member_pointer_v<F>
        constexpr decltype(auto) operator()(T&&... call_args) && noexcept(std::is_nothrow_invocable_v<F, A, T...>) {
            return std::invoke(Functor, std::move(arg), std::forward<T>(call_args)...);
        }

    private:
        A arg;
    };

    template <auto Functor, typename A, typename B>
        requires HasFunctionCallOperator<decltype(Functor)> || Member<decltype(Functor)>
    struct ConstBindingArgumentStorage<Functor, A, B> {
        using F = decltype(Functor);

        template <typename T, typename U>
            requires std::convertible_to<T, A> && std::convertible_to<U, B>
        constexpr explicit ConstBindingArgumentStorage(T &&arg1, U&& arg2) : arg1(std::forward<T>(arg1)), arg2(std::forward<U>(arg2)) {
        }

        template <typename... T>
            requires std::invocable<F, T..., A&, B&>
        constexpr decltype(auto) operator()(T&&... call_args) & noexcept(std::is_nothrow_invocable_v<F, T..., A&, B&>) {
            return std::invoke(Functor, std::forward<T>(call_args)..., arg1, arg2);
        }

        template <typename... T>
            requires std::invocable<F, T..., const A&, const B&>
        constexpr decltype(auto) operator()(T&&... call_args) & noexcept(std::is_nothrow_invocable_v<F, T..., const A&, const B&>) {
            return std::invoke(Functor, std::forward<T>(call_args)..., arg1, arg2);
        }

        template <typename... T>
            requires std::invocable<F, T..., A, B>
        constexpr decltype(auto) operator()(T&&... call_args) && noexcept(std::is_nothrow_invocable_v<F, T..., A, B>) {
            return std::invoke(Functor, std::forward<T>(call_args)..., std::move(arg1), std::move(arg2));
        }

        template <typename... T>
            requires std::invocable<F, A&, T..., B&> && std::is_member_pointer_v<F>
        constexpr decltype(auto) operator()(T&&... call_args) & noexcept(std::is_nothrow_invocable_v<F, A&, T..., B&>) {
            return std::invoke(Functor, arg1, std::forward<T>(call_args)..., arg2);
        }

        template <typename... T>
            requires std::invocable<F, const A&, T..., const B&> && std::is_member_pointer_v<F>
        constexpr decltype(auto) operator()(T&&... call_args) & noexcept(std::is_nothrow_invocable_v<F, const A&, T..., const B&>) {
            return std::invoke(Functor, arg1, std::forward<T>(call_args)..., arg2);
        }

        template <typename... T>
            requires std::invocable<F, A, T..., B> && std::is_member_pointer_v<F>
        constexpr decltype(auto) operator()(T&&... call_args) && noexcept(std::is_nothrow_invocable_v<F, A, T..., B>) {
            return std::invoke(Functor, std::move(arg1), std::forward<T>(call_args)..., std::move(arg2));
        }

    private:
        A arg1;
        B arg2;
    };

    template <auto Functor, typename... A>
        requires HasFunctionCallOperator<decltype(Functor)> || Member<decltype(Functor)>
    struct FunctorBinding {
        using F = decltype(Functor);
        using Storage = ConstBindingArgumentStorage<Functor, A...>;



        template <typename... T>
            requires std::constructible_from<Storage, T...> && (!PackSameAs<FunctorBinding, T...>)
        constexpr explicit FunctorBinding(T&&... extra_args) : args(std::forward<T>(args)) {}

        template <typename... T>
            requires std::invocable<Storage, T...>
        constexpr decltype(auto) operator()(T&&... extra_args) & noexcept(std::is_nothrow_invocable_v<Storage&, T...>) {
            return args(std::forward<T>(extra_args)...);
        }

        template <typename... T>
            requires std::invocable<Storage, T...>
        constexpr decltype(auto) operator()(T&&... extra_args) const & noexcept(std::is_nothrow_invocable_v<const Storage&, T...>) {
            return args(std::forward<T>(extra_args)...);
        }

        template <typename... T>
            requires std::invocable<Storage, T...>
        constexpr decltype(auto) operator()(T&&... extra_args) && noexcept(std::is_nothrow_invocable_v<Storage&&, T...>) {
            return std::move(args)(std::forward<T>(extra_args)...);
        }

    private:
        Storage args;
    };

}
