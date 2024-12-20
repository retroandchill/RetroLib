/**
 * @file Iterators.h
 * @brief TODO: Fill me out
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */

#pragma once

#if !RETROLIB_WITH_MODULES
#include <ranges>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro {
    template <typename I>
    concept SinglePassIterator = std::input_or_output_iterator<I> && !std::forward_iterator<I>;
}