/**
 * @file TypeTraits.h
 * @brief Additional enhancements to the type_traits library
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */

#pragma once

#if !RETROLIB_WITH_MODULES
#include <type_traits>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro {

    /**
     * @struct InvalidType
     *
     * @brief Represents an invalid type with a static validity check.
     *
     * This struct is used as a marker to indicate that a certain type or operation is invalid.
     * The static member `IsValid` is always set to `false`, signifying that this type does
     * not meet the necessary criteria or conditions to be considered valid within the context
     * where it's used.
     *
     * @details
     * This struct can be utilized in template metaprogramming to enforce type constraints.
     * It can serve as a default type, representing an invalid or unsupported type scenario,
     * allowing developers to easily determine and handle such cases in their code logic.
     *
     * @see IsValid
     */
    RETROLIB_EXPORT struct InvalidType {
        static constexpr bool is_valid = false;
    };

    /**
     * @struct ValidType
     *
     * @brief Represents a type that is confirmed to be valid.
     *
     * This structure contains a static constant expression that is used
     * to determine the validity of the type. It can be used in template
     * metaprogramming to conditionally compile code based on the validity
     * of the type.
     */
    RETROLIB_EXPORT struct ValidType {
        static constexpr bool is_valid = true;
    };


}
