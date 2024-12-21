/**
 * @file OrElseThrow.h
 * @brief Get the value contained within an optional, otherwise throw a std::bad_optional_access.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/Functional/CreateBinding.h"
#include "RetroLib/Functional/ExtensionMethods.h"
#include "RetroLib/Optionals/OptionalOperations.h"
#include "RetroLib/Optionals/Optional.h"
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro::optionals {

    /**
     * Evaluates the given optional value and, in case the value does not exist,
     * invokes a functor with the provided arguments to throw an exception.
     *
     * @param optional The optional value to be checked. Must be an object that supports `has_value` and `get` functions.
     * @param functor The callable object that, when invoked, produces or throws an exception.
     * @param args The arguments to be forwarded to the functor when it is invoked.
     * @return The contained value of the optional object if a value is present.
     *         If no value is present, this function throws the result of invoking the functor.
     *
     * @throw The exception produced by invoking the functor with the provided arguments.
     */
    RETROLIB_EXPORT template <OptionalType O, typename F, typename... A>
        requires std::invocable<F, A...> && std::derived_from<std::invoke_result_t<F, A...>, std::exception>
    constexpr decltype(auto) or_else_throw(O&& optional, F&& functor, A&&... args) {
        if (has_value(std::forward<O>(optional))) {
            return get<O>( std::forward<O>(optional));
        }

        throw std::invoke(std::forward<F>(functor), std::forward<A>(args)...);
    }

    /**
     * Provides a utility to handle an optional value or throw if the value is not present.
     *
     * This function forwards an optional value and additional arguments to another overload
     * that binds arguments to a specified functor. This can be used to throw an exception
     * or handle the missing value in a custom way.
     *
     * @param optional An optional value to be checked.
     * @param args Additional arguments to be forwarded and bound to a functor.
     * @return The resulting value if the optional contains a value, or an exception is thrown
     *         if the optional is empty.
     */
    RETROLIB_EXPORT template <auto Functor, OptionalType O, typename... A>
        requires std::invocable<decltype(Functor), A...> && std::derived_from<std::invoke_result_t<decltype(Functor), A...>, std::exception>
    constexpr decltype(auto) or_else_throw(O&& optional, A&&... args) {
        return or_else_throw(std::forward<O>(optional), bind_back<Functor>(std::forward<A>(args)...));
    }

    /**
     * Attempts to retrieve the value from the given optional object. If the optional is not engaged,
     * it throws an exception generated by the provided exception factory.
     *
     * @param optional The optional object to retrieve the value from. Must be an rvalue reference.
     * @return The value contained within the optional if it is engaged.
     * @throws std::bad_optional_access if the optional is not engaged.
     */
    RETROLIB_EXPORT template <OptionalType O>
    constexpr decltype(auto) or_else_throw(O&& optional) {
        return or_else_throw(std::forward<O>(optional), [] { return std::bad_optional_access{}; });
    }

    struct OrElseThrowInvoker {

        template <OptionalType O>
        constexpr decltype(auto) operator()(O&& optional) const {
            return or_else_throw(std::forward<O>(optional));
        }

        template <OptionalType O, typename F, typename... A>
            requires std::invocable<F, A...> && std::derived_from<std::invoke_result_t<F, A...>, std::exception>
        constexpr decltype(auto) operator()(O&& optional, F&& functor, A&&... args) const {
            return or_else_throw(std::forward<O>(optional), std::forward<F>(functor), std::forward<A>(args)...);
        }

    };

    constexpr OrElseThrowInvoker or_else_throw_invoker;

    template <auto Functor>
    struct ConstOrElseThrowInvoker {
        template <OptionalType O, typename... A>
            requires std::invocable<decltype(Functor), A...> && std::derived_from<std::invoke_result_t<decltype(Functor), A...>, std::exception>
        constexpr decltype(auto) operator()(O&& optional, A&&... args) const {
            return or_else_throw<Functor>(std::forward<O>(optional), std::forward<A>(args)...);
        }
    };

    template <auto Functor>
    constexpr ConstOrElseThrowInvoker<Functor> const_or_else_throw_invoker;

    /**
     * This method invokes the `or_else_throw` operation on an optional value. If the optional
     * contains a valid value, the method will pass on that value. If the optional contains no value,
     * it throws an exception, which could be customized by providing an optional set of arguments.
     *
     * @param args Variadic arguments used to customize the behavior of the exception thrown.
     *             These could include a callable or details about the exception to be thrown.
     * @return Returns the result of invoking the `or_else_throw` operation via the
     *         `extension_method` for the provided arguments.
     */
    RETROLIB_EXPORT template <typename... A>
    constexpr auto or_else_throw(A&&... args) {
        return extension_method<or_else_throw_invoker>(std::forward<A>(args)...);
    }

    /**
     * Utility function that invokes an operation on an optional value,
     * throwing an exception if the value is not present.
     *
     * @tparam A Variadic template for the arguments to be forwarded to the lambda or invoker.
     * @param args Arguments to be forwarded to the invoker function associated with the operation.
     * @return The result of the operation performed by the invoker if the optional value is present.
     *         Throws an exception if the optional value is not present. The type of the exception depends
     *         on the invoker or lambda used.
     */
    RETROLIB_EXPORT template <auto Functor, typename... A>
    constexpr auto or_else_throw(A&&... args) {
        return extension_method<const_or_else_throw_invoker<Functor>>(std::forward<A>(args)...);
    }

}