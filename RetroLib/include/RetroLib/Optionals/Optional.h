/**
 * @file Optional.h
 * @brief Custom optional type that has support for reference-based optionals.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/Optionals/OptionalOperations.h"
#include "RetroLib/Concepts/Comparisons.h"
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro {

    /**
     * @struct IsValidOptionalType
     * @brief A type trait used to determine if a type is a valid optional type.
     *
     * This struct inherits from std::true_type, indicating that the default
     * case for this trait is true. It provides a mechanism to validate
     * types as optional, allowing for compile-time checks.
     *
     * Users can extend or specialize this struct to modify the behavior
     * for specific types, ensuring that custom types can be identified as
     * valid optional types when necessary.
     *
     * @note Specialize this struct for any custom type that should be
     *       considered as a valid optional type in your implementation.
     */
    template <typename>
    struct IsValidOptionalType : std::true_type {};

    /**
     * @struct IsValidOptionalType
     *
     * @brief Trait class template specialization to determine the validity of a type as an optional.
     *
     * This specialization of the `IsValidOptionalType` template is specifically for handling
     * `std::nullopt_t`, a type used to represent disengaged states in optional objects.
     * For this specialization, it derives from `std::false_type`, indicating that
     * `std::nullopt_t` is not a valid type for an optional object in this context.
     *
     * @tparam std::nullopt_t Specialized parameter representing the type indicating
     *         an uninitialized or empty state in optional objects.
     */
    template <>
    struct IsValidOptionalType<std::nullopt_t> : std::false_type {};

    /**
     * @brief Struct template specialization to define the validity of an optional type.
     *
     * This specialization of the `IsValidOptionalType` struct template is used to
     * mark `std::in_place_type_t<T>` as an invalid optional type. It inherits from
     * `std::false_type`, effectively indicating a logical "false" for checks that
     * rely on this struct to determine type validity.
     *
     * @tparam T The type encapsulated by `std::in_place_type_t`.
     */
    template <typename T>
    struct IsValidOptionalType<std::in_place_type_t<T>> : std::false_type {};

    /**
     * Concept to check if a type is allowed to be used in an optional.
     *
     * @tparam T The target type
     */
    RETROLIB_EXPORT template <typename T>
    concept ValidOptionalType = IsValidOptionalType<std::decay_t<T>>::value;

    /**
     * @brief A struct to provide optional storage capabilities for an object of type T.
     *
     * The OptionalStorage struct manages an object's lifetime, ensuring it is well-constructed
     * and destroyed only when it contains a value. This provides a way to have an object in a
     * semi-default, inactive state using the standard library's `std::optional`.
     *
     * @tparam T The type of the object to be stored in the optional.
     */
    template <ValidOptionalType T>
    struct OptionalStorage {
        /**
         * @brief Constructs an OptionalStorage object in an empty state.
         *
         * The constructor initializes the OptionalStorage instance to represent an
         * empty state, effectively wrapping a `std::nullopt`. This indicates that
         * the OptionalStorage does not currently hold a value. The constructor is
         * marked `noexcept`, guaranteeing that no exceptions will be thrown during
         * its execution.
         */
        constexpr OptionalStorage() noexcept : empty(std::nullopt) {}

        /**
         * @brief Copy constructs an OptionalStorage object.
         *
         * This constructor initializes the OptionalStorage instance by copying
         * the state from another OptionalStorage object. The operation requires
         * that the type `T` is copy constructible.
         *
         * @param other The OptionalStorage object to copy from. If `other` contains
         * a value, the new object will also contain a copied instance of that value.
         * The `is_set` flag is used to determine if the contained value should be copied.
         *
         * @return A new OptionalStorage instance with a copied value if `other` contains one,
         * otherwise an empty OptionalStorage.
         */
        constexpr OptionalStorage(const OptionalStorage& other) requires std::is_copy_constructible_v<T> : is_set(other.is_set) {
            if (is_set) {
                new (&value) T(other.value);
            }
        }

        /**
         * @brief Move constructor for OptionalStorage.
         *
         * This constructor creates an OptionalStorage object by moving the state from another
         * OptionalStorage instance. It transfers ownership of the value from the `other`
         * OptionalStorage when it is set. The operation is `noexcept` if the type `T` is
         * nothrow move constructible. This constructor requires that the type `T` is
         * move constructible.
         *
         * @param other The OptionalStorage instance to move from. After the operation,
         * the `other` instance does not guarantee the preservation of its original value.
         *
         * @return A new OptionalStorage instance with the value moved from `other` if
         * it was set; otherwise, an empty state.
         */
        constexpr OptionalStorage(OptionalStorage&& other) noexcept(std::is_nothrow_move_constructible_v<T>) requires std::is_move_constructible_v<T> : is_set(other.is_set) {
            if (is_set) {
                new (&value) T(std::move(other.value));
            }
        }

        /**
         * @brief Constructs an OptionalStorage object by copying from another OptionalStorage.
         *
         * This constructor initializes the OptionalStorage instance by copying the state from
         * another OptionalStorage object of a potentially different type. It will copy the contained
         * value if the other OptionalStorage is set. The use of `explicit` ensures that implicit
         * conversions are restricted based on the convertibility of the contained types, enforcing
         * explicit initialization if necessary.
         *
         * @param other The other OptionalStorage object from which to copy the value and state.
         *              If the `other` is set, its value will be copied into this OptionalStorage.
         *              Otherwise, this OptionalStorage will remain in the unset state.
         * @return A new instance of OptionalStorage that mirrors the value and state of the
         *         provided `other` OptionalStorage.
         */
        template <typename U>
            requires std::constructible_from<T, const U&>
        constexpr explicit(!std::convertible_to<const U&, T>) OptionalStorage(const OptionalStorage<U>& other) : is_set(other.is_set) {
            if (is_set) {
                new (&value) T(other.value);
            }
        }

        /**
         * @brief Constructs an OptionalStorage object by moving from another OptionalStorage.
         *
         * This constructor initializes the OptionalStorage instance by moving the value
         * from another OptionalStorage object of potentially different type `U`. The
         * initialization is conditional on whether the source OptionalStorage is set.
         * If it is set, the value is moved into this new OptionalStorage. The explicit
         * specifier ensures that the constructor is only invoked when the conversion
         * is not implicitly possible, providing safer type conversions.
         *
         * @param other The source OptionalStorage object from which to move the value.
         *        It must be an rvalue reference to ensure that its resources can be moved.
         *
         * @return A new instance of OptionalStorage with the moved value from the
         *         other OptionalStorage if it was set, otherwise an empty state.
         */
        template <typename U>
            requires std::constructible_from<T, U&&>
        constexpr explicit(!std::convertible_to<U&&, T>) OptionalStorage(OptionalStorage<U>&& other) : is_set(other.is_set) {
            if (is_set) {
                new (&value) T(std::move(other.value));
            }
        }

        /**
         * @brief Constructs an OptionalStorage object with a value in place.
         *
         * This constructor initializes the OptionalStorage instance to hold a value
         * of type `T`, constructed in place with the provided arguments. The constructor
         * uses `std::in_place_type_t` to indicate direct construction of the underlying
         * type `T` using perfect forwarding for the given arguments.
         *
         * @param args The arguments to forward to the constructor of `T` for in-place construction.
         */
        template <typename... A>
            requires std::constructible_from<T, A...>
        constexpr explicit OptionalStorage(std::in_place_type_t<T>, A&&... args) : value(std::forward<A>(args)...), is_set(true) {
        }

        /**
         * @brief Constructs an OptionalStorage object directly with a value.
         *
         * This constructor initializes the OptionalStorage instance to contain a value
         * constructed in-place. It utilizes an initializer list and additional
         * arguments to directly construct the value of type `T`. The `is_set` flag is
         * set to true, indicating that the OptionalStorage holds a value.
         *
         * @param initializer_list An initializer list of type `U`, used to construct
         *        the contained value.
         * @param args Additional arguments used to construct the contained value.
         */
        template <typename U, typename... A>
            requires std::constructible_from<T, std::initializer_list<U>, A...>
        constexpr explicit OptionalStorage(std::in_place_type_t<T>, std::initializer_list<U> initializer_list, A&&... args)  : value(std::move(initializer_list), std::forward<A>(args)...), is_set(true) {}

        /**
         * @brief Constructs an OptionalStorage object holding a specified value.
         *
         * This constructor initializes an OptionalStorage instance to hold a specified
         * value of type U, which is convertible to type T. The internal state is marked
         * as set, indicating that the OptionalStorage contains a valid value.
         *
         * @param value The value to be stored in the OptionalStorage. It is forwarded
         *              to initialize the internal value of type T.
         */
        template <typename U = T>
            requires std::constructible_from<T, U>
        constexpr explicit(std::convertible_to<U, T>) OptionalStorage(U&& value) : value(std::forward<U>(value)), is_set(true) {
        }

        /**
         * @brief Destructor for the OptionalStorage object.
         *
         * This destructor ensures proper cleanup of the stored value if it is set and
         * the type `T` is not trivially destructible. If the OptionalStorage is in a
         * state where it holds a value, the destructor explicitly calls the destructor
         * of `T` to release resources. If `T` is trivially destructible, then no action
         * is taken as the compiler can handle trivial destruction automatically.
         */
        ~OptionalStorage() {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                if (is_set) {
                    value.~T();
                }
            }
        }

        /**
         * @brief Copy-assigns the value from another OptionalStorage instance.
         *
         * This operator performs a copy assignment from another OptionalStorage object.
         * It requires that the contained type T is copy-assignable. If both instances
         * have a set value, it copies the value. If only the other instance has a set
         * value, it constructs a new value. If neither have a set value, no operation
         * occurs aside from setting the state. If T is not trivially copyable and the
         * current instance has a set value, it ensures proper destruction of the
         * current value before assignment.
         *
         * @param other The OptionalStorage object to copy the value from.
         * @return A reference to the current instance after assignment.
         */
        constexpr OptionalStorage& operator=(const OptionalStorage& other) requires std::is_copy_assignable_v<T> {
            if (is_set && other.is_set) {
                value = other.value;
            } else if (other.is_set) {
                new (&value) T(other.value);
            }  else {
                if constexpr (!std::is_trivially_copyable_v<T>) {
                    if (is_set) {
                        value.~T();
                    }
                }
            }

            is_set = other.is_set;
            return *this;
        }

        /**
         * @brief Move assignment operator for OptionalStorage.
         *
         * This operator allows the transfer of resources from one OptionalStorage
         * instance to another using move semantics. The state of the source instance
         * (`other`) is transferred to the target instance (`*this`). The operation is
         * `noexcept` if the type `T` is nothrow move assignable. This function is
         * constrained to require that `T` is move assignable.
         *
         * @param other The OptionalStorage instance to move from. After the operation,
         *              `other` may be in a valid but unspecified state.
         * @return A reference to the updated OptionalStorage instance.
         */
        constexpr OptionalStorage& operator=(OptionalStorage&& other) noexcept(std::is_nothrow_move_assignable_v<T>) requires std::is_move_assignable_v<T> {
            if (is_set && other.is_set) {
                value = std::move(other.value);
            } else if (other.is_set) {
                new (&value) T(std::move(other.value));
            } else {
                if constexpr (!std::is_trivially_copyable_v<T>) {
                    if (is_set) {
                        value.~T();
                    }
                }
            }

            is_set = other.is_set;
            return *this;
        }

        /**
         * @brief Assigns a new value to the OptionalStorage object.
         *
         * This operator is used to assign a new value to an existing OptionalStorage
         * instance. If the current instance already holds a value (indicated by `is_set`),
         * the existing value is replaced with the new value provided. If it does not hold
         * a value, a new value is constructed in place using the provided value.
         *
         * @param other An rvalue reference to the new value to be assigned. The value is
         * perfect-forwarded to the assignment operator.
         * @return A reference to the OptionalStorage instance after assignment.
         */
        template <typename U = T>
            requires std::assignable_from<T, U>
        constexpr OptionalStorage& operator=(U&& other) {
            if (is_set) {
                value = std::forward<U>(other);
            } else {
                new (&value) T(std::forward<U>(other));
            }

            is_set = other.is_set;
            return *this;
        }

        /**
         * @brief Assigns the value from another OptionalStorage to this instance.
         *
         * This operator overload allows the assignment of a value from another
         * OptionalStorage object. If both objects contain a value, the current value
         * is replaced with the other object's value. If the other object is empty and
         * the current one has a value, the value is destroyed. In case the current
         * object is empty and the other contains a value, a new value is constructed
         * in place.
         *
         * @param other The OptionalStorage object from which to assign the value.
         * @return A reference to this OptionalStorage instance after the assignment.
         */
        template <typename U = T>
            requires std::assignable_from<T, const U&>
        constexpr OptionalStorage& operator=(const OptionalStorage<U>& other) {
            if (is_set && other.is_set) {
                value = other.value;
            } else if (other.is_set) {
                new (&value) T(other.value);
            }  else {
                if constexpr (!std::is_trivially_copyable_v<T>) {
                    if (is_set) {
                        value.~T();
                    }
                }
            }

            is_set = other.is_set;
            return *this;
        }

        /**
         * @brief Move assignment operator for OptionalStorage.
         *
         * This operator assigns the value from another OptionalStorage object by moving
         * it. If both the current instance and the other instance have set values, the
         * value is moved directly. If only the other instance has a set value, the value
         * is constructed in place. If neither instance has a set value, any existing value
         * in the current instance is destroyed, provided the type T is not trivially copyable.
         *
         * @param other An rvalue reference to the OptionalStorage object whose value is
         * to be moved. The other object will be left in a valid but unspecified state.
         * @return A reference to this OptionalStorage instance containing the result of
         * the assignment.
         */
        template <typename U = T>
            requires std::assignable_from<T, U&&>
        constexpr OptionalStorage& operator=(OptionalStorage<U>&& other) {
            if (is_set && other.is_set) {
                value = other.value;
            } else if (other.is_set) {
                new (&value) T(other.value);
            }  else {
                if constexpr (!std::is_trivially_copyable_v<T>) {
                    if (is_set) {
                        value.~T();
                    }
                }
            }

            is_set = other.is_set;
            return *this;
        }

        /**
         * @brief Retrieves a modifiable reference to the stored value.
         *
         * This function returns a reference to the contained value, allowing for
         * modifications. It provides direct access to the stored object within the
         * instance when called on an lvalue. The caller assumes responsibility
         * for ensuring that the object is in a valid, non-empty state before invoking
         * this function.
         *
         * @return A reference to the stored value of type `T`.
         */
        constexpr T& get_value() & {
            return value;
        }

        /**
         * @brief Retrieves the stored value as a constant reference.
         *
         * This function returns a constant reference to the value currently held by
         * the object. It is intended for use when the object is known to be in a state
         * where it contains a valid value, allowing callers to access the value without
         * modifying it.
         *
         * @return A constant reference to the stored value.
         */
        constexpr const T& get_value() const & {
            return value;
        }

        /**
         * @brief Retrieves and moves the stored value from the object.
         *
         * This method provides access to the stored value, transferring ownership
         * by moving it out of the object. The method is invoked on an rvalue reference
         * to the object, indicating that the value will no longer be needed in its
         * original location. The use of `std::move` ensures that the value is moved
         * rather than copied, optimizing for performance in scenarios where the
         * contained type `T` supports move semantics.
         *
         * @return A rvalue reference to the stored value, `T&&`, effectively transferring
         * the ownership to the caller.
         */
        constexpr T&& get_value() && {
            return std::move(value);
        }

        /**
         * @brief Checks whether a value is present in the OptionalStorage.
         *
         * This function returns a boolean indicating whether the OptionalStorage
         * contains a valid value. The function evaluates the internal state to
         * determine if a value has been set.
         *
         * @return `true` if a value is set and present, `false` otherwise.
         */
        constexpr bool has_value() const {
            return is_set;
        }

        /**
         * @brief Swaps the contents of this OptionalStorage with another.
         *
         * This method exchanges the stored value and state of the current
         * OptionalStorage instance with those of the `other` instance.
         * The swap operation is only performed if at least one
         * instance is set. If both instances are in an unset state, no actions
         * are taken, leaving both instances unchanged. The swap is designed to
         * be noexcept provided the type `T` satisfies the conditions of being
         * nothrow move-constructible and nothrow swappable.
         *
         * @param other The other OptionalStorage instance with which to swap contents.
         */
        constexpr void swap(OptionalStorage& other) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_swappable_v<T>) {
            if (!is_set && !other.is_set) {
                return;
            }

            if (is_set && !other.is_set) {
                other.value = std::move(value);
                value.~T();
                is_set = false;
                other.is_set = true;
            } else if (!is_set) {
                value = std::move(other.value);
                other.value.~T();
                other.is_set = false;
                is_set = true;
            } else {
                std::swap(value, other.value);
            }
        }

        /**
         * @brief Resets the state of the object.
         *
         * This method resets the object to an empty state. If the contained type `T`
         * is not trivially destructible and the object currently holds a value,
         * the destructor of the contained value is explicitly called. After this
         * operation, the flag indicating whether the object holds a value is set
         * to `false`.
         *
         * @note This method is `constexpr`, allowing it to be evaluated at compile
         * time if the conditions for `constexpr` evaluation are met.
         */
        constexpr void reset() {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                if (has_value()) {
                    value.~T();
                }
            }

            is_set = false;
        }

        /**
         * @brief Places a new value in the storage with given arguments.
         *
         * The `emplace` method constructs a new object of type `T` using the provided
         * arguments and stores it in the internal storage. If a value is already present,
         * it is overwritten with the new value. The internal flag `is_set` is updated to
         * indicate that the storage now holds a valid object.
         *
         * @param args The arguments to be forwarded to the constructor of `T`.
         */
        template <typename... A>
            requires std::constructible_from<T, A...>
        constexpr void emplace(A&&... args) {
            if (is_set) {
                value = T(std::forward<A>(args)...);
            } else {
                new (&value) T(std::forward<A>(args)...);
            }

            is_set = true;
        }

        /**
         * @brief Reinitializes the stored value with a new instance constructed using an initializer list and additional arguments.
         *
         * This method constructs a new instance of type T using the provided initializer list and additional variadic arguments,
         * replacing the existing value if there is one. If the current optional state is not set, it constructs the new instance in-place
         * in the storage.
         *
         * @param initializer_list The initializer list to be used for constructing a new instance of type T.
         * @param args Additional arguments to be passed to the constructor of type T.
         */
        template <typename U, typename... A>
            requires std::constructible_from<T, std::initializer_list<U>, A...>
        constexpr void emplace(std::initializer_list<U> initializer_list, A&&... args) {
            if (is_set) {
                value = T(std::move(initializer_list), std::forward<A>(args)...);
            } else {
                new (&value) T(std::move(initializer_list), std::forward<A>(args)...);
            }

            is_set = true;
        }

        /**
         * @brief Compares two OptionalStorage objects for equality.
         *
         * This operator checks whether both OptionalStorage instances are in the same state.
         * It returns true if both instances do not hold a value or if both hold values that are equal.
         *
         * @tparam U The type of the value stored in the OptionalStorage.
         * @param other The OptionalStorage instance to compare with this instance.
         * @return true if both instances are either empty or hold equal values, otherwise false.
         */
        template <typename U>
            requires EqualityComparable<T, U>
        constexpr bool operator==(const OptionalStorage<U>& other) const {
            if (!is_set && !other.is_set) {
                return true;
            }

            if (is_set && other.is_set) {
                return value == other.value;
            }

            return false;
        }

        /**
         * @brief Performs three-way comparison between this OptionalStorage and another.
         *
         * This operator function implements a three-way comparison logic between two
         * OptionalStorage objects. It utilizes the spaceship operator `<=>` to compare
         * the stored values if both instances are set. The comparison adheres to the
         * following rules:
         *
         * - If neither instance has a set value, they are considered equivalent.
         * - If both instances possess values, a direct comparison of those values is conducted.
         * - If only this instance has a value, it is considered greater than the other.
         * - If only the other instance has a value, this instance is considered less.
         *
         * The result is a value of type `std::compare_three_way_result_t<T, U>`, which
         * encapsulates the outcome of the comparison in terms of equivalence, greater,
         * or less.
         *
         * @param other The other OptionalStorage instance to compare against.
         * @return The result of the three-way comparison, indicating equivalence,
         * greater, or lesser outcome.
         */
        template <typename U>
            requires ThreeWayComparable<T, U>
        constexpr std::compare_three_way_result_t<T, U> operator<=>(const OptionalStorage<U>& other) const {
            if (!is_set && !other.is_set) {
                return std::compare_three_way_result_t<T, U>::equivalent;
            }

            if (is_set && other.is_set) {
                return value <=> other.value;
            }

            if (is_set && !other.is_set) {
                return std::compare_three_way_result_t<T, U>::greater;
            }

            return std::compare_three_way_result_t<T, U>::less;
        }

        /**
         * @brief Compares the stored value with another value for equality.
         *
         * This operator checks whether the OptionalStorage object is in a set state
         * and compares the stored value with the value provided as an argument.
         * The comparison is only performed if the OptionalStorage object currently
         * holds a value.
         *
         * @param other The value to compare with the stored value.
         * @return true if the OptionalStorage is set and the stored value is equal
         * to the specified value, otherwise false.
         */
        template <typename U>
            requires EqualityComparable<T, U>
        constexpr bool operator==(const U& other) const {
            if (!is_set) {
                return false;
            }

            return value == other;
        }

        /**
         * @brief Compares the stored value with another value using three-way comparison.
         *
         * This operator performs a three-way comparison between the stored value and
         * the provided value of type `U`. If the stored value is not set, it returns
         * `std::compare_three_way_result_t<T, U>::less`, indicating that the current
         * object is considered less than the provided value. If the stored value is
         * set, the comparison is delegated to the underlying values using the spaceship
         * operator `<=>`.
         *
         * @param other A reference to the value of type `U` to be compared with the
         * stored value.
         * @return The result of the three-way comparison as
         * `std::compare_three_way_result_t<T, U>`.
         */
        template <typename U>
            requires ThreeWayComparable<T, U>
        constexpr std::compare_three_way_result_t<T, U> operator<=>(const U& other) const {
            if (!is_set) {
                return std::compare_three_way_result_t<T, U>::less;
            }

            return value <=> other;
        }

    private:
        template <ValidOptionalType U>
        friend struct OptionalStorage;

        union {
            std::nullopt_t empty;
            T value;
        };
        bool is_set = false;
    };

    /**
     * @brief A class template that represents an optional object, which may or may not contain a value.
     *
     * The Optional class is a versatile utility that encapsulates an optional value. It provides various
     * constructors, assignment operators, and utility functions to manage the optional value. The class
     * is designed with a focus on safety and flexibility, utilizing various conditionally available
     * constructors and operators based on the value type characteristics.
     *
     * @tparam T The type of the value that may be optionally stored.
     */
    RETROLIB_EXPORT template <ValidOptionalType T>
    class Optional {
    public:
        /**
         * @brief Default constructor for Optional.
         *
         * Constructs an Optional object in a default state. The constructor is
         * marked as `constexpr` and `noexcept`, ensuring that the construction
         * can be evaluated at compile-time and no exceptions will be thrown.
         *
         * @return An Optional instance in an uninitialized state.
         */
        constexpr Optional() noexcept = default;

        /**
         * @brief Constructs an Optional object in an empty state using std::nullopt.
         *
         * This constructor initializes the Optional instance to an empty or uninitialized
         * state, equivalent to not holding any meaningful value. Being marked `constexpr`,
         * it allows the initialization to occur at compile time, and `noexcept` ensures
         * no exceptions are thrown during its operation.
         *
         * @param std::nullopt_t A tag indicating that the Optional should be initialized
         *                       in an empty state.
         * @return An Optional object in an empty state.
         */
        constexpr explicit Optional(std::nullopt_t) noexcept {}

        /**
         * @brief Copy constructor for an Optional object.
         *
         * This constructor allows for the creation of an Optional instance
         * by copying another Optional object, provided the type `T` is
         * copy constructible. The use of `constexpr` and `default`
         * indicates that the constructor will offer compile-time performance
         * enhancements and will be automatically generated by the compiler.
         *
         * @param other The Optional instance to copy.
         * @return A new Optional object that is a copy of the specified instance.
         */
        constexpr Optional(const Optional&) requires std::is_copy_constructible_v<T> = default;

        /**
         * @brief Move constructor for Optional, conditionally noexcept.
         *
         * This constructor enables the move semantics for an Optional object, provided
         * that the type `T` it holds is move constructible. The noexcept specification
         * is applied conditionally based on whether the move constructor of `T` is noexcept.
         * This behavior ensures efficient transfer of resources from one Optional instance
         * to another with guarantees regarding exception safety, specifically propagating
         * the noexcept property of the contained type.
         *
         * @param other The Optional instance to be moved from. After the move, `other`
         * may not hold a valid value.
         * @return A new Optional instance initialized with the moved value from `other`.
         */
        constexpr Optional(Optional&&)noexcept(std::is_nothrow_move_constructible_v<T>) requires std::is_move_constructible_v<T> = default;

        /**
         * @brief Constructs an Optional object from another Optional instance with a different type.
         *
         * This constructor initializes the Optional instance by converting the contained value
         * from another Optional object of a potentially different type. The conversion is only
         * explicit if the type U is not implicitly convertible to T. The operation will
         * effectively copy the value if the other Optional holds one, otherwise it will
         * represent the empty state.
         *
         * This constructor is declared constexpr, meaning it can be evaluated at compile time
         * if all its arguments are also constant expressions.
         *
         * @tparam U Type of the value in the other Optional instance.
         * @param other Another instance of Optional to initialize from.
         */
        template <typename U>
            requires std::constructible_from<T, const U&>
        constexpr explicit(!std::convertible_to<const U&, T>) Optional(const Optional<U>& other) : storage(other.storage) {}

        /**
         * @brief Constructs an Optional object by transferring ownership from another Optional.
         *
         * This constructor initializes the Optional instance by transferring the contents of
         * another Optional of a possibly different type. The conversion is only allowed if the
         * type `U&&` is explicitly convertible to type `T`. The use of `std::move` ensures that
         * the resources from the `other` Optional are safely transferred, leaving it in a valid
         * but unspecified state.
         *
         * @tparam U The type of the Optional object from which to construct.
         * @param other The Optional instance from which to transfer ownership.
         */
        template <typename U>
            requires std::constructible_from<T, U&&>
        constexpr explicit(!std::convertible_to<U&&, T>) Optional(Optional<U>&& other) : storage(std::move(other.storage)) {}

        /**
         * @brief Constructs an Optional object with an in-place initialized value.
         *
         * This constructor initializes the Optional object with a value of type `T`,
         * using the provided arguments to construct the value directly in place. It
         * makes use of the `std::in_place_type` mechanism to enable direct construction
         * of the contained object.
         *
         * @param args The arguments forwarded to the constructor of the contained
         * object of type `T`.
         */
        template <typename... A>
            requires std::constructible_from<T, A...>
        constexpr explicit Optional(std::in_place_type_t<T>, A&&... args) : storage(std::in_place_type<T>, std::forward<A>(args)...) {}

        /**
         * @brief Constructs an Optional object in an initialized state.
         *
         * This constructor initializes the Optional instance to hold a value of type T
         * by directly constructing it with a specified initializer list and additional
         * arguments. This allows for the construction of types that support initializer
         * list initialization.
         *
         * @param initializer_list An initializer list to be used in constructing the
         * value of type T.
         * @param args Additional arguments to be used in the construction of the
         * value of type T.
         *
         * @tparam U The type elements within the initializer list.
         * @tparam A A variadic template for any additional arguments required for
         * constructing the object of type T.
         */
        template <typename U, typename... A>
            requires std::constructible_from<T, std::initializer_list<U>, A...>
        constexpr explicit Optional(std::in_place_type_t<T>, std::initializer_list<U> initializer_list, A&&... args) : storage(std::in_place_type<T>, std::move(initializer_list), std::forward<A>(args)...) {}

        /**
         * @brief Constructs an Optional object with a specified value.
         *
         * This constructor initializes the Optional instance with a given value,
         * effectively storing the value within the `storage` member. The constructor
         * does not throw exceptions.
         *
         * @param value The value to initialize the Optional instance with.
         * @return An Optional object encapsulating the provided value.
         */
        constexpr explicit(false) Optional(const T& value) : storage(value) {}

        /**
         * @brief Constructs an Optional object with a given value.
         *
         * This constructor initializes the Optional instance to hold a specific value,
         * copying or moving the given value into its internal storage. The use of
         * `constexpr` allows for the possibility of compile-time evaluation.
         *
         * @tparam T The type of the value to be stored in the Optional object.
         * @param value The value to be stored, which will be moved into the Optional.
         */
        constexpr explicit(false) Optional(T&& value) : storage(std::move(value)) {}

        /**
         * @brief Constructs an Optional object by forwarding a value of type U.
         *
         * This constructor initializes the Optional object using perfect forwarding
         * of a value of type U. The `explicit` specifier is conditionally applied
         * based on whether type U is convertible to type T, ensuring that implicit
         * conversions happen only when safe. This constructor provides a way to
         * directly initialize the Optional with a given value.
         *
         * @tparam U The type of the value being forwarded.
         * @param value The value to be forwarded and stored in the Optional.
         */
        template <ValidOptionalType U = T>
            requires std::constructible_from<T, U>
        constexpr explicit(!std::convertible_to<U, T>) Optional(U&& value) : storage(std::forward<U>(value)) {}

        /**
         * @brief Default destructor for the Optional class.
         *
         * This destructor is implicitly `noexcept` and will clean up any resources
         * associated with an instance of the Optional class. If the Optional holds
         * an object, the destructor will call the destructor of the held object
         * appropriately.
         */
        ~Optional() = default;

        /**
         * @brief Assignment operator for copying an Optional object.
         *
         * This operator allows for the assignment of one Optional instance to another when
         * the type T is copy assignable. It uses the default copy assignment behavior provided
         * by the compiler, ensuring efficient and reliable copying of Optional objects.
         *
         * @param other The Optional instance to be copied from.
         * @return A reference to the current Optional instance after the assignment.
         */
        constexpr Optional& operator=(const Optional&) requires std::is_copy_assignable_v<T> = default;

        /**
         * @brief Assigns a new value by moving from another Optional object.
         *
         * This move assignment operator transfers the contents from the given Optional
         * source object to the current instance. The move is only permitted if the template
         * type `T` is move assignable. The operation is marked `noexcept` if the type `T`
         * provides a noexcept move assignment.
         *
         * @param rhs The right-hand side Optional object to be moved.
         * @return A reference to the current Optional object after the move assignment.
         */
        constexpr Optional& operator=(Optional&&) noexcept(std::is_nothrow_move_assignable_v<T>) requires std::is_move_assignable_v<T> = default;

        /**
         * @brief Assigns a state of null optional to the Optional object.
         *
         * This operator overload facilitates the assignment of `std::nullopt` to
         * an Optional instance. Upon invocation, it resets the internal storage
         * to signify an empty state. The function returns a reference to the
         * current object after the assignment is complete, allowing for chaining
         * of operations.
         */
        constexpr Optional& operator=(std::nullopt_t) {
            storage.reset();
            return *this;
        }

        /**
         * @brief Assigns a value to the Optional object.
         *
         * This assignment operator replaces the current contents of the Optional object
         * with the specified value. The operation uses forwarding to accept both lvalue
         * and rvalue references, allowing perfect forwarding of the provided value.
         *
         * @tparam U The type of the value being assigned to the Optional.
         * @param value The value to be assigned, which is forwarded internally.
         * @return A reference to the Option object after the assignment.
         */
        template <typename U = T>
            requires std::assignable_from<T, U>
        constexpr Optional& operator=(U&& value) {
            storage = std::forward<U>(value);
            return *this;
        }

        /**
         * @brief Assigns the contents of another Optional object to this instance.
         *
         * This assignment operator replaces the current storage of this Optional object
         * with the storage from another Optional object of potentially different type U.
         * The assigned Optional object is directly copied into this instance's storage,
         * ensuring that both Optionals will subsequently contain equivalent values.
         *
         * @tparam U The type of the Optional object from which the value is being
         * assigned.
         * @param other The Optional object to be assigned to the current instance.
         * @return A reference to the updated Optional instance.
         */
        template <typename U = T>
            requires std::assignable_from<T, const U&>
        constexpr Optional& operator=(const Optional<U>& other) {
            storage = other.storage;
            return *this;
        }

        /**
         * @brief Move assignment operator for Optional.
         *
         * This operator allows moving the contents of another Optional object into the
         * current instance. It transfers ownership of the underlying storage from the
         * provided Optional object to this one, effectively assigning the moved state
         * and invalidating the source Optional.
         *
         * @param other An rvalue reference to the Optional object whose contents are
         * to be moved into this Optional instance.
         * @return A reference to the current Optional instance after the assignment.
         */
        template <typename U = T>
            requires std::assignable_from<T, U&&>
        constexpr Optional& operator=(Optional<U>&& other) {
            storage = std::move(other.storage);
            return *this;
        }

        /**
         * @brief Dereferences the storage to retrieve the contained value.
         *
         * This operator provides access to the value stored within the storage
         * object when it is in a valid state. The method applies when the
         * OptionalStorage instance is an lvalue reference. It utilizes `get_value()`
         * to fetch the stored value, potentially throwing an exception if the storage
         * does not contain a valid value.
         *
         * @return A reference to the stored value.
         */
        constexpr decltype(auto) operator*() & {
            return storage.get_value();
        }

        /**
         * @brief Dereferences the OptionalStorage to access the stored value.
         *
         * This operator provides access to the value contained within the OptionalStorage
         * when it is called on a constant reference. It returns the result of invoking
         * `get_value()` on the internal storage, which retrieves the stored value.
         * The use of `decltype(auto)` allows the operator to return the exact type
         * returned by `get_value()`.
         */
        constexpr decltype(auto) operator*() const & {
            return storage.get_value();
        }

        /**
         * @brief Overloaded dereference operator for rvalue instances.
         *
         * This operator allows access to the stored value of the `storage` object
         * when the instance is an rvalue. It returns the value by moving it,
         * enabling optimal performance for temporary objects.
         *
         * @return The moved value stored within the `storage`.
         */
        constexpr decltype(auto) operator*() && {
            return std::move(storage.get_value());
        }

        /**
         * @brief Provides access to the stored value via a pointer.
         *
         * This operator returns a pointer to the value stored within, allowing access
         * to the value's members as if it were a pointer. The operator assumes that the
         * storage currently holds a valid value, and accessing the value is safe.
         *
         * @return A pointer to the stored value, facilitating member access.
         */
        constexpr decltype(auto) operator->() {
            return &storage.get_value();
        }

        /**
         * @brief Provides access to the stored value's pointer.
         *
         * The operator accesses the encapsulated value and returns a pointer
         * to it. This enables pointer-like semantics for objects that internally
         * manage a value.
         *
         * @return A pointer to the contained value.
         */
        constexpr decltype(auto) operator->() const {
            return &storage.get_value();
        }

        /**
         * @brief Checks if the OptionalStorage contains a value.
         *
         * This function evaluates whether the OptionalStorage instance currently holds
         * a value. It returns a boolean indicating the presence (true) or absence (false)
         * of a stored value within the OptionalStorage.
         *
         * @return True if there is a value stored, false otherwise.
         */
        constexpr bool has_value() const {
            return storage.has_value();
        }

        /**
         * @brief Retrieves the stored value with non-const lvalue reference.
         *
         * This function returns a non-const lvalue reference to the stored value if the object
         * contains a value. If the object is in an empty state, it throws a `std::bad_optional_access`
         * exception. The function utilizes `decltype(auto)` to preserve the exact type and value category
         * of the return value, ensuring correct forwarding semantics.
         *
         * @throws std::bad_optional_access if no value is stored.
         *
         * @return Non-const lvalue reference to the contained value.
         */
        constexpr decltype(auto) value() & {
            if (!has_value()) {
                throw std::bad_optional_access();
            }

            return storage.get_value();
        }

        /**
         * @brief Accesses the contained value.
         *
         * This function returns a reference to the value held by the object.
         * It assumes that the object contains a valid value. If the object
         * does not hold a value, it throws a `std::bad_optional_access` exception.
         *
         * @return Reference to the contained value.
         *
         * @throws std::bad_optional_access Thrown if the object is in an empty state
         * and does not contain a valid value.
         */
        constexpr decltype(auto) value() const & {
            if (!has_value()) {
                throw std::bad_optional_access();
            }

            return storage.get_value();
        }

        /**
         * @brief Retrieves the stored value when the object is an r-value.
         *
         * This method returns the contained value by moving it, only if the object
         * currently holds a value. If the object does not contain a value, it throws
         * a `std::bad_optional_access` exception, indicating an attempt to access
         * the value of an empty state. This function is marked `constexpr` to allow
         * evaluation at compile time if possible.
         *
         * @return A rvalue-reference to the stored value.
         *
         * @throws std::bad_optional_access If the object does not hold a value.
         */
        constexpr decltype(auto) value() && {
            if (!has_value()) {
                throw std::bad_optional_access();
            }

            return std::move(storage.get_value());
        }

        /**
         * @brief Returns the stored value if available, otherwise returns a provided default value.
         *
         * This function checks whether the object holds a value. If a value is present, it is returned.
         * Otherwise, the provided default value is returned. This method does not modify the state of the object.
         *
         * @param default_value The value to return if no stored value is available. It is a forwarding
         * reference, allowing for efficient passing of temporary or lvalue references.
         *
         * @return The stored value if present, otherwise the provided default value.
         */
        template <typename U>
            requires std::convertible_to<U, T>
        constexpr T value_or(U&& default_value) const & {
            if (has_value()) {
                return storage.get_value();
            }

            return std::forward<U>(default_value);
        }

        template <typename U>
            requires std::convertible_to<U, T>
        /**
         * @brief Returns the contained value or a default value.
         *
         * This method checks if the object currently holds a value. If it does, the
         * contained value is returned by moving it. If no value is present, the
         * specified default value is returned. This method is designed to work with
         * rvalue-qualified objects.
         *
         * @param default_value The default value to return if the object does not hold a value.
         * @return The contained value if present, otherwise the provided default value.
         */
        constexpr T value_or(U&& default_value) && {
            if (has_value()) {
                return std::move(storage.get_value());
            }

            return std::forward<U>(default_value);
        }

        /**
         * @brief Swaps the contents of this Optional object with another.
         *
         * This method exchanges the internal storage of this Optional instance with
         * that of the `other` Optional instance. The operation follows the `noexcept`
         * specification, only throwing exceptions if either the type T's move
         * constructor or its swap operation can throw. This ensures that the swap
         * operation is safe and efficient, assuming that the type T supports nothrow
         * move construction and swapping.
         *
         * @param other The Optional instance to swap contents with.
         */
        constexpr void swap(Optional& other) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_swappable_v<T>) {
            storage.swap(other.storage);
        }

        /**
         * @brief Resets the storage to an empty state.
         *
         * This function resets the internal storage, effectively removing any current value
         * and setting it to an empty state. It is a `constexpr` and `noexcept` function,
         * ensuring compile-time evaluation where possible and that no exceptions will be thrown
         * during its execution.
         */
        constexpr void reset() noexcept {
            storage.reset();
        }

        /**
         * @brief Constructs a new value in-place within the storage.
         *
         * This function initializes a new value directly within the storage using
         * the provided arguments. It effectively forwards the given arguments
         * to the constructor of the value type, allowing for efficient emplacement
         * without creating temporary objects. This operation will replace any
         * existing value held in the storage.
         *
         * @tparam A Variadic template parameters for the constructor arguments.
         * @param args Arguments to be forwarded to the constructor of the value type.
         */
        template <typename... A>
            requires std::constructible_from<T, A...>
        constexpr void emplace(A&&... args) {
            storage.emplace(std::forward<A>(args)...);
        }

        /**
         * @brief Constructs a new value in-place using an initializer list.
         *
         * This function utilizes an initializer list and additional arguments to
         * construct a new value directly within the storage. The method ensures that
         * the existing contents are destroyed and replaced by the new value.
         *
         * @param initializer_list The initializer list used for the construction of the new value.
         * @param args Additional arguments to be forwarded to the constructor.
         */
        template <typename U, typename... A>
            requires std::constructible_from<T, std::initializer_list<U>, A...>
        constexpr void emplace(std::initializer_list<U> initializer_list, A&&... args) {
            storage.emplace(std::move(initializer_list), std::forward<A>(args)...);
        }

        /**
         * @brief Compares two Optional objects for equality.
         *
         * This operator checks if the current Optional object is equal to another
         * Optional object by comparing their underlying storage. It returns true if
         * both objects have identical storage contents, indicating that they either
         * both hold no value or hold the same value. This operation is performed in a
         * constexpr context, allowing for evaluation at compile-time if the conditions
         * are met.
         *
         * @param other The Optional object to compare against.
         * @return True if both Optional objects are equal based on their storage;
         * false otherwise.
         */
        template <typename U>
			requires EqualityComparable<T, U>
        constexpr bool operator==(const Optional<U>& other) const {
            return storage == other.storage;
        }

        /**
         * @brief Performs a three-way comparison between the current Optional object and another.
         *
         * This operator provides a three-way comparison, utilizing the spaceship operator
         * (`<=>`), between the internal storage of the current Optional object and that of
         * another Optional object. It determines the ordering based on the values stored
         * within the two objects.
         *
         * @param other The Optional object to compare against the current instance.
         * @return The result of the three-way comparison, encapsulated in
         * `std::compare_three_way_result_t<T, U>`, providing information about the relative
         * ordering of the two Optional objects.
         */
        template <typename U>
			requires ThreeWayComparable<T, U>
        constexpr std::compare_three_way_result_t<T, U> operator<=>(const Optional<U>& other) const {
            return storage <=> other.storage;
        }

        /**
         * @brief Compares the OptionalStorage object with a nullopt.
         *
         * This operator checks whether the OptionalStorage instance is currently
         * in an empty state, equivalent to being compared with `std::nullopt`.
         *
         * @param std::nullopt_t A special placeholder used to denote an empty state in optional types.
         * @return true if the OptionalStorage does not hold a value, indicating an empty state;
         *         otherwise, false.
         */
        constexpr bool operator==(std::nullopt_t) const {
            return !has_value();
        }

        /**
         * @brief Compares an OptionalStorage object with a nullopt_t to determine ordering.
         *
         * This operator provides a comparison between the OptionalStorage instance and
         * a `std::nullopt_t` using three-way comparison. It indicates greater ordering
         * if the OptionalStorage currently holds a value, otherwise, it returns equal
         * ordering to signify an empty state.
         *
         * @param none The function does not take any parameters as it operates on the
         * instance it belongs to, and compares internally with `std::nullopt_t`.
         *
         * @return std::strong_ordering::greater if the OptionalStorage has a value,
         *         std::strong_ordering::equal if it represents a null state.
         */
        constexpr std::strong_ordering operator<=>(std::nullopt_t) const {
            return has_value() ? std::strong_ordering::greater : std::strong_ordering::equal;
        }

        /**
         * @brief Compares the current object with another object for equality.
         *
         * This operator checks if the current object's storage is equal to the
         * provided `other` object. The comparison is done using the equality
         * operator for the underlying type.
         *
         * @param other The object to compare against the current storage.
         * @return `true` if the current storage is equal to `other`, `false` otherwise.
         */
        template <typename U>
            requires EqualityComparable<T, U>
        constexpr bool operator==(const U& other) const {
            return storage == other;
        }

        /**
         * @brief Compares the stored value with another value using three-way comparison.
         *
         * This operator performs a three-way comparison between the stored value and
         * the specified `other` parameter. It returns the result of the comparison
         * using `std::compare_three_way_result_t`, which provides a consistent
         * ordering relationship. The comparison is based on the spaceship operator
         * (`<=>`), which evaluates whether the stored value is less than, equal to, or
         * greater than the `other` value.
         *
         * @tparam U The type of the value to compare with.
         * @param other The value to be compared against the stored value.
         * @return A result indicating if the stored value is less, equal, or greater than `other`.
         */
        template <typename U>
            requires ThreeWayComparable<T, U>
        constexpr std::compare_three_way_result_t<T, U> operator<=>(const U& other) const {
            return storage <=> other;
        }

    private:
        template <ValidOptionalType U>
        friend class Optional;

        OptionalStorage<T> storage;
    };
}

namespace std {
    /**
     * @brief Swaps the values of two Optional objects.
     *
     * This function exchanges the contents of two `retro::Optional` instances.
     * It is marked `constexpr` to allow for compile-time execution and `noexcept`
     * to ensure that it does not throw exceptions, provided that the swap operation
     * on the underlying objects is also `noexcept`.
     *
     * @tparam T The type of the value stored in the Optional objects.
     * @param lhs The first Optional object to swap.
     * @param rhs The second Optional object to swap.
     */
    template <retro::ValidOptionalType T>
        requires std::is_move_constructible_v<T> && std::is_swappable_v<T>
    constexpr void swap(retro::Optional<T>& lhs, retro::Optional<T>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
        lhs.swap(rhs);
    }
}