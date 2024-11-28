/**
 * @file OpaqueStorage.h
 * @brief Contains the concept definitions for small buffer optimization with opaque storage unions.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#ifndef RETROLIB_EXPORT
#include "RetroLib/Concepts/Inheritance.h"
#include <array>

#define RETROLIB_EXPORT
#endif

namespace Retro {
    /**
     * Global constant used to determine the size of the small buffer for opaque storage types.
     */
    RETROLIB_EXPORT constexpr size_t SMALL_STORAGE_SIZE = sizeof(void *) * 7;

    /**
     * @brief Concept that verifies if a type can fit into small storage.
     *
     * The `CanFitSmallStorage` concept checks whether the size of a given type `T` is less than
     * or equal to the predefined constant `SMALL_STORAGE_SIZE`. This allows for constraints
     * on template parameters to ensure that only types fitting within a certain size are considered.
     *
     * @tparam T The type to be checked.
     */
    RETROLIB_EXPORT template<typename T>
    concept CanFitSmallStorage = sizeof(T) <= SMALL_STORAGE_SIZE;

}
