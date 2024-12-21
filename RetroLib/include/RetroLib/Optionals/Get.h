/**
 * @file Get.h
 * @brief Functional chain binding for getting an optional's value.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/Functional/ExtensionMethods.h"
#include "RetroLib/Optionals/OptionalOperations.h"
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro::optionals {

    struct GetInvoker {

        template <OptionalType O>
        constexpr decltype(auto) operator()(O&& optional) const {
            RETROLIB_ASSERT(has_value(std::forward<O>(optional)), "Cannot get value from an empty optional");
            return get(std::forward<O>(optional));
        }

    };

    /**
     * Retrieves a value by invoking the extension method specified by GetInvoker.
     *
     * @return The result of the extension_method invocation for GetInvoker.
     */
    RETROLIB_EXPORT constexpr auto get() {
        return extension_method<GetInvoker{}>();
    }

}
