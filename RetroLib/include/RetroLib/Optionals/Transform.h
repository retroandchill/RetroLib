/**
 * @file Transform.h
 * @brief Transform operation for optionals.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */

#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/Functional/CreateBinding.h"
#include "RetroLib/FunctionTraits.h"

#include <ranges>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro::optionals {

    RETROLIB_EXPORT template <OptionalType O, typename... A>
            requires std::is_invocable_v<BindingType<A...>, CommonReference<O>>
        constexpr auto transform(O &&optional, A &&...args) {
        if constexpr (OptionalReference<O>) {
            return has_value(std::forward<O>(optional)) ?
        }
    }

}