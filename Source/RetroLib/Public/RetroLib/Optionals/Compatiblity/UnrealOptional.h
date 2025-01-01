// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#ifdef __UNREAL__
#include <RetroLib/Optionals/OptionalIterator.h>

#include "RetroLib/Optionals/Optional.h"
#include "RetroLib/Utils/ForwardLike.h"

namespace Retro::Optionals {
    template <typename T>
    concept UnrealInstrusiveUnsetState = ValidOptionalType<T> && std::movable<T> && std::constructible_from<T, FIntrusiveUnsetOptionalState> && requires(const T& Value, FIntrusiveUnsetOptionalState Unset) {
        { Value == Unset } -> std::convertible_to<bool>;
        { Value != Unset } -> std::convertible_to<bool>;
    };

    template <typename>
    struct TUnsetBridgeTag {};
    
}

template <typename T, bool HasIntrusiveUnsetOptionalState>
struct UE::Core::Private::TOptionalStorage<Retro::Optionals::TUnsetBridgeTag<T>, HasIntrusiveUnsetOptionalState> {
    static FIntrusiveUnsetOptionalState CreateUnset() {
        return FIntrusiveUnsetOptionalState();
    };
};

template <Retro::Optionals::UnrealInstrusiveUnsetState T>
struct Retro::AlternateIntrusiveUnsetState<T> {
    static T Construct() {
        return T(UE::Core::Private::TOptionalStorage<Optionals::TUnsetBridgeTag<T>, true>::CreateUnset());
    }

    static bool IsUnset(const T& Value) {
        return Value != (UE::Core::Private::TOptionalStorage<Optionals::TUnsetBridgeTag<T>, true>::CreateUnset());
    }
};

namespace Retro::Optionals {
    
    template <typename T>
    concept UnrealOptional = requires(T &&Optional) {
        Optional.GetValue();
        *Optional;
        Optional.operator->();
        { Optional.IsSet() } -> std::same_as<bool>;
    };

    template <typename>
    struct TUeOptional : InvalidType {};

    template <typename T>
    struct TUeOptional<TOptional<T>> : ValidType {
        using Type = T;
    };

    template <typename T>
        requires TUeOptional<std::decay_t<T>>::IsValid
    using TUeOptionalType = typename TUeOptional<std::decay_t<T>>::Type;
    

    template <UnrealOptional T>
    struct OptionalOperations<T> : ValidType {
        template <UnrealOptional O>
            requires std::same_as<T, std::decay_t<O>>
        static constexpr decltype(auto) Get(O &&Optional) {
            if constexpr (std::is_lvalue_reference_v<TUeOptionalType<O>>) {
                return *Optional;
            } else {
                return Retro::ForwardLike<O>(*Optional);
            }
        }

        template <UnrealOptional O>
            requires std::same_as<T, std::decay_t<O>>
        static constexpr decltype(auto) GetValue(O &&Optional) {
            if constexpr (std::is_lvalue_reference_v<TUeOptionalType<O>>) {
                return Optional.GetValue();
            } else {
                return Retro::ForwardLike<O>(Optional.GetValue());
            }
        }

        template <UnrealOptional O>
            requires std::same_as<T, std::decay_t<O>>
        static constexpr bool HasValue(const O &Optional) {
            return Optional.IsSet();
        }
    };

    template <typename T>
        requires std::derived_from<std::remove_cv_t<T>, UObject>
    struct NullableOptionalParam<TObjectPtr<T>> : ValidType {
        using RawType = T;
        using ReferenceType = std::add_lvalue_reference_t<T>;

        template <template <typename...> typename O, typename U>
            requires std::assignable_from<ReferenceType, U&> && OptionalType<O<std::reference_wrapper<U>>>
        static constexpr auto OfNullable(const TObjectPtr<U> &Ptr) {
            if constexpr (RawReferenceOptionalValid<O, U>) {
                return Ptr != nullptr ? O<ReferenceType>(*Ptr) : O<ReferenceType>();
            } else {
                return Ptr != nullptr ? O<std::reference_wrapper<T>>(*Ptr) : O<std::reference_wrapper<T>>();
            }
        }
    };

    template <typename T>
        requires SpecializationOf<std::decay_t<T>, TObjectPtr> && (!std::same_as<std::decay_t<T>, T>)
    struct NullableOptionalParam<T> : NullableOptionalParam<std::decay_t<T>> {
    };
} // namespace retro::optionals

/**
 * Template specialization for an optional that takes in a reference.
 * @tparam T The type of the optional that was consumed.
 */
template <typename T>
struct TOptional<T &> {

    /**
     * Typedef for the element type used by this container.
     */
    using ElementType = T;

    /**
     * Default constructor, creates an empty optional.
     */
    constexpr TOptional() = default;

    /**
     * Construct a new optional from a reference.
     * @param Value The value to reference in this optional.
     */
    constexpr explicit(false) TOptional(T &Value) : Data(&Value) {
    }

    /**
     * Construct a new optional from a nullable pointer.
     * @param Value
     */
    constexpr explicit(false) TOptional(T *Value) : Data(Value) {
    }

    /**
     * Constructor from nullptr
     */
    constexpr explicit(false) TOptional(nullptr_t) {
    }

    /**
     * Construct a new optional from an optional of subclass.
     * @tparam U The subclass type
     * @param Other The other optional
     */
    template <typename U>
        requires std::derived_from<U, T> && (!std::is_same_v<T, U>)
    constexpr explicit(false) TOptional(TOptional<U &> Other) : Data(Other.GetPtrOrNull()) {
    }

    /**
     * Assignment operator from the underlying value.
     * @param Value The value to assign in.
     * @return A reference to this object
     */
    TOptional &operator=(T &Value) {
        Data = &Value;
        return *this;
    }

    /**
     * Emplace a new reference value into the optional.
     * @param Value The value to emplace
     * @return A reference to the emplaced value
     */
    T &Emplace(T &Value) {
        Data = Value;
        return *Data;
    }

    /**
     * Get the value of the optional.
     * @param DefaultValue The default to substitute for an empty optional.
     * @return The retrieved value
     */
    T &Get(T &DefaultValue) const
        requires(!std::is_const_v<T>)
    {
        return Data != nullptr ? *Data : DefaultValue;
    }

    /**
     * Get the value of the optional.
     * @param DefaultValue The default to substitute for an empty optional.
     * @return The retrieved value
     */
    const T &Get(const T &DefaultValue) const {
        return Data != nullptr ? *Data : DefaultValue;
    }

    /**
     * Get a nullable pointer for the optional.
     * @return The retrieved value
     */
    T *GetPtrOrNull() const {
        return Data;
    }

    /**
     * Get a reference to the underlying data.
     * @return The retrieved value
     */
    T &GetValue() const {
        check(Data != nullptr) return *Data;
    }

    /**
     * Returns if the value is set
     * @return Is there a valid optional?
     */
    bool IsSet() const {
        return Data != nullptr;
    }

    /**
     * Reset the data to empty.
     */
    void Reset() {
        Data = nullptr;
    }

    /**
     * Dereference operator.
     * @return A reference to the underlying data.
     */
    T &operator*() {
        check(Data != nullptr) return *Data;
    }

    /**
     * Dereference operator.
     * @return A reference to the underlying data.
     */
    T &operator*() const {
        check(Data != nullptr) return *Data;
    }

    /**
     * Pointer member access operator.
     * @return A pointer to the underlying data.
     */
    T *operator->() {
        check(Data != nullptr) return Data;
    }

    /**
     * Pointer member access operator.
     * @return A pointer to the underlying data.
     */
    T *operator->() const {
        check(Data != nullptr) return Data;
    }

  private:
    T *Data = nullptr;
};

template <>
struct Retro::Optionals::IsRawReferenceOptionalAllowed<TOptional> : std::true_type {};

template <typename T>
constexpr auto begin(TOptional<T>& Optional) {
    return Retro::Optionals::OptionalIterator(Optional);
}

template <typename T>
constexpr auto begin(const TOptional<T>& Optional) {
    return Retro::Optionals::OptionalIterator(Optional);
}

template <typename T>
[[noreturn]] constexpr auto begin(TOptional<T>&&) {
    static_assert(false, "Cannot iterate over an r-value TOptional value");
}

template <typename T>
constexpr auto end(const TOptional<T>&) {
    return Retro::Optionals::OptionalSentinel();
}
#endif