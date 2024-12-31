/**
 * @file RttiCheck.h
 * @brief Contains the basic info for checking if RTTI is enabled.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include <cassert>
#endif

#if defined(__clang__)
#if __has_feature(cxx_rtti)
#define RTTI_ENABLED
#endif
#elif defined(__GNUG__)
#if defined(__GXX_RTTI)
#define RTTI_ENABLED
#endif
#elif defined(_MSC_VER)
#if defined(_CPPRTTI)
#define RTTI_ENABLED
#endif
#endif

#define RETROLIB_ASSERT(...) assert(__VA_ARGS__)

#define RETROLIB_FUNCTIONAL_EXTENSION(Exporter, Method, Name) \
  constexpr auto Invoker_##Name##_Method_Variable = Method; \
  template <auto Functor = DynamicFunctor> \
        requires (DynamicFunctorBinding<Functor> || IsValidFunctorObject(Functor)) \
  constexpr FunctorBindingInvoker<Functor, Invoker_##Name##_Method_Variable> FunctorExtension_##Name##_; \
  Exporter template <auto Functor = DynamicFunctor, typename... A> \
        requires (DynamicFunctorBinding<Functor> || IsValidFunctorObject(Functor)) \
    constexpr auto Name(A &&...Args) { \
          return ExtensionMethod<FunctorExtension_##Name##_<Functor>>(std::forward<A>(Args)...); \
    }