/**
 * @file Nullability.h
 * @brief Concepts for handling nullability.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/TypeTraits.h"
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro {

    RETROLIB_EXPORT template <typename>
    struct NullableTraits : InvalidType {};

    RETROLIB_EXPORT template <typename T>
        requires std::is_pointer_v<T>
    struct NullableTraits<T> : ValidType {
        using RawType = std::remove_pointer_t<T>;
        using ReferenceType = std::add_lvalue_reference_t<RawType>;
    };

    RETROLIB_EXPORT template <template <typename...> typename C, typename T>
        requires std::convertible_to<C<T>, std::add_pointer_t<T>>
    struct NullableTraits<C<T>> : ValidType {
        using RawType = T;
        using ReferenceType = std::add_lvalue_reference_t<RawType>;
    };

    RETROLIB_EXPORT template <typename T>
    concept NullableType = NullableTraits<std::decay_t<T>>::is_valid;

    RETROLIB_EXPORT template <NullableType T>
    using PointedType = typename NullableTraits<T>::RawType;

    RETROLIB_EXPORT template <NullableType T>
    using ReferenceType = typename NullableTraits<T>::ReferenceType;


}