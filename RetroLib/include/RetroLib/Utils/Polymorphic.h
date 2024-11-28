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
#include "RetroLib/Utils/OpaqueStorage.h"
#include <typeinfo>

#define RETROLIB_EXPORT
#endif

namespace Retro {

    RETROLIB_EXPORT template <ClassType T>
    class Polymorphic {
      public:
        constexpr Polymorphic() noexcept requires std::is_default_constructible_v<T> {
            storage.emplace<T>();
        }

        template <typename U>
            requires std::derived_from<std::decay_t<U>, T>
        explicit constexpr Polymorphic(U&& value) noexcept : vtable(getVTable<U>()) {
            storage.emplace<std::decay_t<U>>(std::forward<U>(value));
        }

        template <typename U, typename... A>
            requires std::derived_from<U, T> && std::constructible_from<U, A...>
        explicit constexpr Polymorphic(std::in_place_type_t<U>, A&&... args) noexcept : vtable(getVTable<U>()) {
            storage.emplace<U>(std::forward<A>(args)...);
        }

        constexpr Polymorphic(const Polymorphic& other) noexcept : vtable(other.vtable) {
            vtable->copy(other.storage, storage);
        }

        constexpr Polymorphic(Polymorphic&& other) noexcept : vtable(other.vtable) {
            vtable->move(other.storage, storage);
        }

        constexpr ~Polymorphic() noexcept {
            vtable->destroy(storage);
        }

        constexpr Polymorphic& operator=(const Polymorphic& other) noexcept {
            if (vtable->getType() == other.vtable->getType()) {
                vtable = other.vtable;
                vtable->copyAssign(other.storage, storage);
            } else {
                vtable->destroy(storage);
                vtable = other.vtable;
                vtable->copy(other.storage, storage);
            }

            return *this;
        }

        constexpr Polymorphic& operator=(const Polymorphic&& other) noexcept {
            if (vtable->getType() == other.vtable->getType()) {
                vtable = other.vtable;
                vtable->moveAssign(other.storage, storage);
            } else {
                vtable->destroy(storage);
                vtable = other.vtable;
                vtable->move(other.storage, storage);
            }
            return *this;
        }

        template <typename U>
            requires std::derived_from<std::decay_t<U>, T>
        constexpr Polymorphic& operator=(U&& value) noexcept {

        }

        constexpr T* get() {
            return vtable->getValue(storage);
        }

        constexpr const T* get() const {
            return vtable->getValue(storage);
        }

        constexpr T* operator->() {
            return get();
        }

        constexpr const T* operator->() const {
            return get();
        }

        constexpr T& operator*() {
            return *get();
        }

        constexpr const T& operator*() const {
            return *get();
        }

      private:
        union OpaqueStorage {
            std::array<std::byte, SMALL_STORAGE_SIZE> smallStorage;
            void *largeStorage;

            template <typename U, typename... A>
                requires std::derived_from<U, T> && std::constructible_from<T, A...>
            constexpr void emplace(A&&... args) noexcept {
                if constexpr (CanFitSmallStorage<T>) {
                    new (reinterpret_cast<T*>(&smallStorage)) T(std::forward<A>(args)...);
                } else {
                    largeStorage = new T(std::forward<A>(args)...);
                }
            }
        };


        struct VTable {
            constexpr virtual ~VTable() = default;
            constexpr virtual const std::type_info& getType() const = 0;
            constexpr virtual T* getValue(OpaqueStorage& storage) const = 0;
            constexpr virtual const T* getValue(const OpaqueStorage& storage) const = 0;
            constexpr virtual void destroy(OpaqueStorage& storage) const = 0;
            constexpr virtual void copy(const OpaqueStorage& src, OpaqueStorage& dest) const = 0;
            constexpr virtual void copyAssign(const OpaqueStorage& src, OpaqueStorage& dest) const = 0;
            constexpr virtual void move(OpaqueStorage& src, OpaqueStorage& dest) const = 0;
            constexpr virtual void moveAssign(OpaqueStorage& src, OpaqueStorage& dest) const = 0;

        };

        template <typename U>
            requires std::derived_from<U, T>
        struct VTableImpl : VTable {
            constexpr const std::type_info & getType() const final {
                return typeid(U);
            }

            constexpr U *getValue(OpaqueStorage &storage) const final {
                if constexpr (CanFitSmallStorage<U>) {
                    return reinterpret_cast<U*>(&storage.smallStorage);
                } else {
                    return static_cast<U*>(storage.largeStorage);
                }
            }

            constexpr const U* getValue(const OpaqueStorage &storage) const final {
                if constexpr (CanFitSmallStorage<U>) {
                    return reinterpret_cast<const U*>(&storage.smallStorage);
                } else {
                    return static_cast<const U*>(storage.largeStorage);
                }
            }

            constexpr void destroy(OpaqueStorage &storage) const final {
                if constexpr (CanFitSmallStorage<U>) {
                    reinterpret_cast<const U*>(&storage.smallStorage)->~U();
                } else {
                    delete static_cast<const U*>(storage.largeStorage);
                }
            }

            constexpr void copy(const OpaqueStorage &src, OpaqueStorage &dest) const final {
                if constexpr (CanFitSmallStorage<U>) {
                    dest.emplace<U>(*reinterpret_cast<const U*>(&src.smallStorage));
                } else {
                    dest.emplace<U>(*static_cast<const U*>(src.largeStorage));
                }
            }

            constexpr void copyAssign(const OpaqueStorage &src, OpaqueStorage &dest) const override {
                if constexpr (CanFitSmallStorage<U>) {
                    *reinterpret_cast<U*>(&dest.smallStorage) = *reinterpret_cast<const U*>(&src.smallStorage);
                } else {
                    *static_cast<U*>(dest.largeStorage) = *static_cast<const U*>(src.largeStorage);
                }
            }

            constexpr void move(OpaqueStorage &src, OpaqueStorage &dest) const final {
                if constexpr (CanFitSmallStorage<U>) {
                    dest.emplace<U>(std::move(*reinterpret_cast<U*>(&src.smallStorage)));
                } else {
                    dest.emplace<U>(std::move(*static_cast<U*>(src.largeStorage)));
                }
            }

            constexpr void moveAssign(OpaqueStorage &src, OpaqueStorage &dest) const override {
                if constexpr (CanFitSmallStorage<T>) {
                    *reinterpret_cast<T*>(&dest.smallStorage) = std::move(*reinterpret_cast<T*>(&src.smallStorage));
                } else {
                    *static_cast<T*>(dest.largeStorage) = std::move(*static_cast<T*>(src.largeStorage));
                }
            }
        };

        template <typename T>
        constexpr static const VTable* getVTable() {
            static constexpr VTableImpl<T> vtable;
            return &vtable;
        }

        OpaqueStorage storage;
        const VTable* vtable = getVTable<T>();
    };

} // Retro
