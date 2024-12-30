/**
 * @file Polymorphic.h
 * @brief Contains the declaration for the polymorphic class.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/Concepts/Inheritance.h"
#include "RetroLib/Concepts/OpaqueStorage.h"
#include "RetroLib/Optionals/Optional.h"

#include <array>
#include <bit>
#include <typeinfo>
#include <utility>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace Retro {
    /**
     * @brief A class template that provides polymorphic storage and access capabilities for types derived from a base
     * class.
     *
     * The Polymorphic class allows storing objects of different derived types within an instance without losing the
     * polymorphic behavior. It provides interfaces to manage the lifetime and access the stored object polymorphically.
     *
     * @tparam T The base class type which all stored objects must derive from. Must satisfy the ClassType concept.
     */
    RETROLIB_EXPORT template <Class T, size_t SmallStorageSize = DEFAULT_SMALL_STORAGE_SIZE>
        requires(SmallStorageSize >= sizeof(void *))
    class Polymorphic {
        template <typename U>
            requires std::derived_from<U, T>
        static constexpr bool FitsSmallStorage = sizeof(U) <= SmallStorageSize;

      public:
        /**
         * Default constructor for the Polymorphic class.
         *
         * This constructor initializes the storage for the Polymorphic object
         * by emplacing an instance of type T. It is only available if T is
         * default constructible.
         *
         * @tparam T The polymorphic type that this class is managing.
         *
         * @pre std::is_default_constructible_v<T> must be true, meaning
         * the type T must have a default constructor.
         *
         * @throws No exceptions are thrown. The constructor is marked noexcept.
         */
        constexpr Polymorphic() noexcept
            requires std::is_default_constructible_v<T>
        {
            Storage.template Emplace<T>();
        }

        /**
         * Constructs a Polymorphic object from a value of type U.
         *
         * This constructor initializes the Polymorphic object using the supplied value,
         * and assigns the appropriate vtable for type U, ensuring polymorphic behavior.
         *
         * @tparam U The type of the value used to construct the Polymorphic object.
         *          It must be a type derived from T.
         *
         * @param Value An instance of type U, which will be stored within the Polymorphic object.
         *
         * @pre U must be derived from T.
         *
         * @post The Polymorphic object is initialized with the given value and the
         *       vtable for type U is set.
         *
         * @throws No exceptions are thrown as the constructor is marked noexcept.
         */
        template <typename U>
            requires std::derived_from<std::decay_t<U>, T>
        explicit(false) constexpr Polymorphic(U &&Value) noexcept : Vtable(GetVtable<U>()) {
            Storage.template Emplace<std::decay_t<U>>(std::forward<U>(Value));
        }

        /**
         * Constructs a Polymorphic object with an in-place type and arguments.
         *
         * This constructor initializes the Polymorphic object by directly constructing
         * an instance of type U within the storage using the provided arguments. It is
         * only available for types U that are derived from T and constructible from
         * the provided argument types A.
         *
         * @tparam U The type of object to be constructed within the Polymorphic. It must
         *        be a type derived from the base type T.
         * @tparam A Parameter pack representing the types of the arguments used for
         *        constructing the U type object.
         *
         * @param Args Arguments to be forwarded to the constructor of U.
         *
         * @pre U must be derived from T and must be constructible from the arguments of types A.
         *
         * @post The Polymorphic object is initialized with an instance of U and the
         *       corresponding vtable for handling polymorphic operations is set.
         *
         * @throws No exceptions are thrown as the constructor is marked noexcept.
         */
        template <typename U, typename... A>
            requires std::derived_from<U, T> && std::constructible_from<U, A...>
        explicit constexpr Polymorphic(std::in_place_type_t<U>, A &&...Args) noexcept : Vtable(GetVtable<U>()) {
            Storage.template Emplace<U>(std::forward<A>(Args)...);
        }

        explicit constexpr Polymorphic(IntrusiveUnsetStateTag) noexcept : Vtable(nullptr) {}

        /**
         * @brief Copy constructor for the Polymorphic class.
         *
         * This constructor initializes a new Polymorphic object as a copy of an existing one.
         * It copies the vtable pointer from the source object and uses the vtable to copy
         * the underlying storage, ensuring that the copied object retains polymorphic behavior
         * identical to the source object.
         *
         * @param Other The Polymorphic object to be copied.
         *
         * @post A new Polymorphic object is created with the same polymorphic type and state
         *       as the object provided.
         *
         * @throws No exceptions are thrown. The constructor is marked noexcept.
         */
        constexpr Polymorphic(const Polymorphic &Other) noexcept : Vtable(Other.Vtable) {
            if (Vtable != nullptr) {
                Vtable->Copy(Other.Storage, Storage);
            }
        }

        /**
         * @brief Move constructor for the Polymorphic class.
         *
         * Constructs a new Polymorphic object by transferring ownership of the
         * resources from an existing Polymorphic object. The vtable is copied from
         * the source object, and the associated move operation defined in the
         * vtable is performed on the source's storage, transferring its contents
         * to the new object's storage.
         *
         * @param Other The Polymorphic object to be moved from. After the move,
         *              the source object is left in a valid but unspecified state.
         *
         * @post The current object is initialized with the transferred state
         *       from `other`, and the vtable pointer is set appropriately.
         *
         * @throws No exceptions are thrown. The constructor is marked noexcept.
         */
        constexpr Polymorphic(Polymorphic &&Other) noexcept : Vtable(Other.Vtable) {
            if (Vtable != nullptr) {
                Vtable->Move(Other.Storage, Storage);
            }
        }

        /**
         * @brief Destructor for the Polymorphic class.
         *
         * This destructor is responsible for properly destroying the dynamically-typed
         * object managed by the Polymorphic instance. It accesses the appropriate destructor
         * function through the vtable and applies it to the internal storage, ensuring
         * that the object is cleaned up correctly according to its actual runtime type.
         *
         * @note This operation is marked as noexcept, indicating that no exceptions will be
         *       thrown during the destruction process.
         */
        constexpr ~Polymorphic() noexcept {
            if (Vtable != nullptr) {
                Vtable->Destroy(Storage);
            }
        }

        /**
         * Assigns the contents of another Polymorphic object to this one.
         *
         * This assignment operator checks whether the dynamic types of the two objects are the same.
         * If they are, it assigns the values by invoking a copy assignment operation through the vtable.
         * If the types differ, it destroys the current contents and performs a new copy via the vtable.
         *
         * @param Other The Polymorphic object to be assigned to the current object.
         * @return A reference to the current object after assignment.
         * @note The operation is noexcept, implying it does not throw exceptions.
         */
        constexpr Polymorphic &operator=(const Polymorphic &Other) noexcept {
            if (Vtable == nullptr) {
                Vtable = Other.Vtable;
                if (Vtable != nullptr) {
                    Vtable->Copy(Other.Storage, Storage);
                }
            } else if (Other.Vtable == nullptr) {
                Vtable->Destroy(Storage);
                Vtable = nullptr;
            } else if (Vtable->GetType() == Other.Vtable->GetType()) {
                Vtable = Other.Vtable;
                Vtable->CopyAssign(Other.Storage, Storage);
            } else {
                Vtable->Destroy(Storage);
                Vtable = Other.Vtable;
                Vtable->Copy(Other.Storage, Storage);
            }

            return *this;
        }

        /**
         * Move assignment operator for the Polymorphic class.
         *
         * This operator allows the transfer of resources from another Polymorphic instance
         * to the current instance, supporting polymorphic behavior through a vtable mechanism.
         * It checks if the current object's type, as defined by its vtable, matches the type
         * of the other object. If so, it directly assigns the vtable and moves the resources.
         * If not, it destroys the current resources, assigns the new vtable, and moves the resources
         * from the other object.
         *
         * @param Other A rvalue reference to the Polymorphic instance being assigned from.
         *
         * @return A reference to the current instance after assignment.
         *
         * @note The operation is noexcept, ensuring that it does not throw exceptions.
         */
        constexpr Polymorphic &operator=(Polymorphic &&Other) noexcept {
            if (Vtable == nullptr) {
                Vtable = Other.Vtable;
                if (Vtable != nullptr) {
                    Vtable->Move(Other.Storage, Storage);
                }
            } else if (Other.Vtable == nullptr) {
                Vtable->Destroy(Storage);
                Vtable = nullptr;
            } else if (Vtable->GetType() == Other.Vtable->GetType()) {
                Vtable = Other.Vtable;
                Vtable->MoveAssign(Other.Storage, Storage);
            } else {
                Vtable->Destroy(Storage);
                Vtable = Other.Vtable;
                Vtable->Move(Other.Storage, Storage);
            }
            return *this;
        }

        /**
         * Assigns a value of a type derived from T to the polymorphic object.
         *
         * This operator allows assigning an object of a type that is derived from the base type T
         * to this polymorphic object. The assignment is only available if U is a derived type
         * of T, as constrained by the std::derived_from requirement.
         *
         * @tparam U A type that is derived from T.
         * @param Value An rvalue reference to the object of type U that is to be assigned.
         * @return A reference to the updated polymorphic object.
         *
         * @note The operation is noexcept, meaning it guarantees not to throw exceptions.
         */
        template <typename U>
            requires std::derived_from<std::decay_t<U>, T>
        constexpr Polymorphic &operator=(U &&Value) noexcept {
            Emplace<std::decay_t<U>>(std::forward<U>(Value));
            return *this;
        }

        constexpr bool operator==(IntrusiveUnsetStateTag) const noexcept {
            return Vtable == nullptr;
        }

        /**
         * Retrieves a pointer to the stored value.
         *
         * This function accesses the stored value through the virtual table's
         * get_value method, returning a pointer of type T.
         *
         * @return A pointer to the stored value of type T.
         */
        constexpr T *Get() {
            return Vtable->GetValue(Storage);
        }

        /**
         * Retrieves a pointer to the stored value.
         *
         * This function accesses the stored value through the virtual table's
         * get_value method, returning a pointer of type T.
         *
         * @return A pointer to the stored value of type T.
         */
        constexpr const T *Get() const {
            return Vtable->GetConstValue(Storage);
        }

        /**
         * Provides access to the underlying pointer.
         *
         * Overloads the arrow operator to allow direct access to the pointer
         * managed by this object. It calls the `get()` method to retrieve the
         * underlying pointer.
         *
         * @return A pointer of type T* managed by this object.
         */
        constexpr T *operator->() {
            return Get();
        }

        /**
         * Provides access to the underlying pointer.
         *
         * Overloads the arrow operator to allow direct access to the pointer
         * managed by this object. It calls the `get()` method to retrieve the
         * underlying pointer.
         *
         * @return A pointer of type T* managed by this object.
         */
        constexpr const T *operator->() const {
            return Get();
        }

        /**
         * Dereference operator that returns a reference to the object managed by the current instance.
         *
         * This operator is used to access the value pointed to by the internal pointer or managed object.
         * It assumes that there is always a valid object to dereference, and it should be used where
         * the certainty of a managed object presence is guaranteed.
         *
         * @return A reference to the object of type T that is managed by the current instance.
         */
        constexpr T &operator*() {
            return *Get();
        }

        /**
         * Dereference operator that returns a reference to the object managed by the current instance.
         *
         * This operator is used to access the value pointed to by the internal pointer or managed object.
         * It assumes that there is always a valid object to dereference, and it should be used where
         * the certainty of a managed object presence is guaranteed.
         *
         * @return A reference to the object of type T that is managed by the current instance.
         */
        constexpr const T &operator*() const {
            return *Get();
        }

        /**
         * Replaces the current object stored in the polymorphic object storage
         * with a new instance of the specified derived type U, constructed
         * with the provided arguments.
         *
         * @tparam U The type of the new object to be stored. Must be a type derived from T.
         * @tparam A The types of the constructor arguments for the new object.
         * @param Args The arguments to be forwarded to the constructor of the new object.
         *
         * @note This function first destroys the current object residing in the storage.
         *       It then checks if the new object fits in small storage. If it does,
         *       the object is constructed in-place within the small storage. Otherwise,
         *       it is allocated in the large storage.
         *
         * @requires U Must be a type derived from T.
         */
        template <typename U, typename... A>
            requires std::derived_from<U, T>
        constexpr void Emplace(A &&...Args) noexcept {
            Vtable->Destroy(Storage);
            Vtable = GetVtable<U>();
            if constexpr (FitsSmallStorage<U>) {
                new (std::bit_cast<U *>(Storage.SmallStorage.data())) U(std::forward<A>(Args)...);
            } else {
                Storage.LargeStorage = new U(std::forward<A>(Args)...);
            }
        }

        /**
         * Retrieves the size from the vtable.
         *
         * @return The size as a constant expression.
         */
        constexpr size_t GetSize() const {
            return Vtable->GetSize();
        }

      private:
        union OpaqueStorage {
            std::array<std::byte, SmallStorageSize> SmallStorage;
            void *LargeStorage;

            template <typename U, typename... A>
                requires std::derived_from<U, T> && std::constructible_from<U, A...>
            constexpr void Emplace(A &&...Args) noexcept {
                if constexpr (FitsSmallStorage<U>) {
                    new (std::bit_cast<U *>(SmallStorage.data())) U(std::forward<A>(Args)...);
                } else {
                    LargeStorage = new U(std::forward<A>(Args)...);
                }
            }
        };

        struct VTable {
            const std::type_info &(*GetType)();
            size_t (*GetSize)();
            T *(*GetValue)(OpaqueStorage &Storage);
            const T *(*GetConstValue)(const OpaqueStorage &Storage);
            void (*Destroy)(OpaqueStorage &Storage);
            void (*Copy)(const OpaqueStorage &Src, OpaqueStorage &Dest);
            void (*CopyAssign)(const OpaqueStorage &Src, OpaqueStorage &Dest);
            void (*Move)(OpaqueStorage &Src, OpaqueStorage &Dest);
            void (*MoveAssign)(OpaqueStorage &Src, OpaqueStorage &Dest);
        };

        template <typename U>
            requires std::derived_from<U, T>
        struct VTableImpl {
            static constexpr const std::type_info &GetType() {
                return typeid(U);
            }

            static constexpr size_t GetSize() {
                return sizeof(U);
            }

            static constexpr T *GetValue(OpaqueStorage &Data) {
                if constexpr (FitsSmallStorage<U>) {
                    return std::bit_cast<U *>(Data.SmallStorage.data());
                } else {
                    return static_cast<U *>(Data.LargeStorage);
                }
            }

            static constexpr const T *GetConstValue(const OpaqueStorage &Data) {
                if constexpr (FitsSmallStorage<U>) {
                    return std::bit_cast<const U *>(Data.SmallStorage.data());
                } else {
                    return static_cast<const U *>(Data.LargeStorage);
                }
            }

            static constexpr void Destroy(OpaqueStorage &Data) {
                if constexpr (FitsSmallStorage<U>) {
                    std::bit_cast<const U *>(Data.SmallStorage.data())->~U();
                } else {
                    delete static_cast<const U *>(Data.LargeStorage);
                }
            }

            static constexpr void Copy(const OpaqueStorage &Src, OpaqueStorage &Dest) {
                if constexpr (FitsSmallStorage<U>) {
                    Dest.template Emplace<U>(*std::bit_cast<const U *>(Src.SmallStorage.data()));
                } else {
                    Dest.template Emplace<U>(*static_cast<const U *>(Src.LargeStorage));
                }
            }

            static constexpr void CopyAssign(const OpaqueStorage &Src, OpaqueStorage &Dest) {
                if constexpr (FitsSmallStorage<U>) {
                    *std::bit_cast<U *>(Dest.SmallStorage.data()) =
                        *std::bit_cast<const U *>(Src.SmallStorage.data());
                } else {
                    *static_cast<U *>(Dest.LargeStorage) = *static_cast<const U *>(Src.LargeStorage);
                }
            }

            static constexpr void Move(OpaqueStorage &Src, OpaqueStorage &Dest) {
                if constexpr (FitsSmallStorage<U>) {
                    Dest.template Emplace<U>(std::move(*std::bit_cast<U *>(Src.SmallStorage.data())));
                } else {
                    Dest.template Emplace<U>(std::move(*static_cast<U *>(Src.LargeStorage)));
                }
            }

            static constexpr void MoveAssign(OpaqueStorage &Src, OpaqueStorage &Dest) {
                if constexpr (FitsSmallStorage<U>) {
                    *std::bit_cast<U *>(Dest.SmallStorage.data()) =
                        std::move(*std::bit_cast<U *>(Src.SmallStorage.data()));
                } else {
                    *static_cast<U *>(Dest.LargeStorage) = std::move(*static_cast<U *>(Src.LargeStorage));
                }
            }
        };

        template <typename U>
            requires std::derived_from<U, T>
        static const VTable *GetVtable() {
            using ImplType = VTableImpl<U>;
            static constexpr VTable Vtable = {.GetType = &ImplType::GetType,
                                              .GetSize = &ImplType::GetSize,
                                              .GetValue = &ImplType::GetValue,
                                              .GetConstValue = &ImplType::GetConstValue,
                                              .Destroy = &ImplType::Destroy,
                                              .Copy = &ImplType::Copy,
                                              .CopyAssign = &ImplType::CopyAssign,
                                              .Move = &ImplType::Move,
                                              .MoveAssign = &ImplType::MoveAssign};
            return &Vtable;
        }

        OpaqueStorage Storage;
        const VTable *Vtable = GetVtable<T>();
    };

} // namespace retro
