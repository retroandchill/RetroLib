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
    RETROLIB_EXPORT constexpr size_t SMALL_STORAGE_SIZE = sizeof(void *) * 7;

    RETROLIB_EXPORT template<typename T>
    concept CanFitSmallStorage = sizeof(T) <= SMALL_STORAGE_SIZE;

}
