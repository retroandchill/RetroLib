/**
 * @file OptionalOperations.h
 * @brief Contains optional utility methods for bridging with other libraries that have their own optional types.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/TypeTraits.h"
#include <optional>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace Retro {

    /**
     * @struct OptionalTraits
     * @brief A traits structure that extends from InvalidType.
     *
     * The OptionalTraits struct is designed to serve as a component in a type traits system.
     * By inheriting from InvalidType, it indicates that it is meant to signify a certain
     * type condition or property within a larger type traits framework.
     *
     * The specific semantics and usage of OptionalTraits are intended to be defined within
     * the context of its integration. It does not inherently possess additional members or
     * functionalities apart from those inherited from InvalidType.
     *
     * In the context where being an InvalidType is meaningful, OptionalTraits may signal
     * or assist in compile-time decision-making or optimizations within generic programming paradigms.
     */
    RETROLIB_EXPORT template <typename>
    struct OptionalTraits : InvalidType {};

    /**
     * @brief A traits structure to specialize behaviors for `std::optional<T>`.
     *
     * This structure inherits from `ValidType`, signifying that `std::optional<T>`
     * meets the criteria of a 'valid' type as defined by the system's type requirements.
     *
     * @tparam T The type contained within the `std::optional` that this traits specialization adapts.
     *
     * The `OptionalTraits` is used to interface with functionalities where knowledge
     * about the type behavior of `std::optional` is essential. This can be part of a larger
     * traits-based system where different types are adapted through their specialized trait structures.
     */
    RETROLIB_EXPORT template <typename T>
    struct OptionalTraits<std::optional<T>> : ValidType {

    };

    RETROLIB_EXPORT template <typename T>
    concept OptionalType = typename OptionalTraits<T>::IsValid;

};