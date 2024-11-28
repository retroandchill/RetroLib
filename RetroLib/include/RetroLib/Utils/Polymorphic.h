/**
 * @file Polymorphic.h
 * @brief Contains the declaration for the polymorphic class.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#ifndef RETROLIB_EXPORT
#include "RetroLib/Concepts/Inheritance.h"

#define RETROLIB_EXPORT
#endif

namespace Retro {

    RETROLIB_EXPORT template <PolymorphicType T>
    class Polymorphic {

    };

} // Retro
