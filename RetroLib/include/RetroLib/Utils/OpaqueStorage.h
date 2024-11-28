/**
 * @file OpaqueStorage.h
 * @brief TODO: Fill me out
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

    union OpaqueStorage {
        std::array<std::byte, SMALL_STORAGE_SIZE> smallStorage;
        void *largeStorage;

        template <typename T, typename... A>
            requires std::constructible_from<T, A...>
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
        constexpr virtual void* getValue(OpaqueStorage& storage) const = 0;
        constexpr virtual const void* getValue(const OpaqueStorage& storage) const = 0;
        constexpr virtual void destroy(OpaqueStorage& storage) const = 0;
        constexpr virtual void copy(const OpaqueStorage& src, OpaqueStorage& dest) const = 0;
        constexpr virtual void copyAssign(const OpaqueStorage& src, OpaqueStorage& dest) const = 0;
        constexpr virtual void move(OpaqueStorage& src, OpaqueStorage& dest) const = 0;
        constexpr virtual void moveAssign(OpaqueStorage& src, OpaqueStorage& dest) const = 0;

    };

    template <typename T>
    struct VTableImpl : VTable {
        constexpr const std::type_info & getType() const final {
            return typeid(T);
        }

        constexpr T *getValue(OpaqueStorage &storage) const final {
            if constexpr (CanFitSmallStorage<T>) {
                return reinterpret_cast<T*>(&storage.smallStorage);
            } else {
                return static_cast<T*>(storage.largeStorage);
            }
        }

        constexpr const T* getValue(const OpaqueStorage &storage) const final {
            if constexpr (CanFitSmallStorage<T>) {
                return reinterpret_cast<const T*>(&storage.smallStorage);
            } else {
                return static_cast<const T*>(storage.largeStorage);
            }
        }

        constexpr void destroy(OpaqueStorage &storage) const final {
            if constexpr (CanFitSmallStorage<T>) {
                reinterpret_cast<const T*>(&storage.smallStorage)->~T();
            } else {
                delete static_cast<const T*>(storage.largeStorage);
            }
        }

        constexpr void copy(const OpaqueStorage &src, OpaqueStorage &dest) const final {
            if constexpr (CanFitSmallStorage<T>) {
                dest.emplace<T>(*reinterpret_cast<const T*>(&src.smallStorage));
            } else {
                dest.emplace<T>(*static_cast<const T*>(src.largeStorage));
            }
        }

        constexpr void copyAssign(const OpaqueStorage &src, OpaqueStorage &dest) const override {
            if constexpr (CanFitSmallStorage<T>) {
                *reinterpret_cast<T*>(&dest.smallStorage) = *reinterpret_cast<const T*>(&src.smallStorage);
            } else {
                *static_cast<T*>(dest.largeStorage) = *static_cast<const T*>(src.largeStorage);
            }
        }

        constexpr void move(OpaqueStorage &src, OpaqueStorage &dest) const final {
            if constexpr (CanFitSmallStorage<T>) {
                dest.emplace<T>(std::move(*reinterpret_cast<T*>(&src.smallStorage)));
            } else {
                dest.emplace<T>(std::move(*static_cast<T*>(src.largeStorage)));
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
    constexpr const VTable* getVTable() {
        static constexpr VTableImpl<T> vtable;
        return &vtable;
    }
}
