/**
 * @file Generator.h
 * @brief Coroutine powered generator type.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/RetroLibMacros.h"

#include <cassert>
#include <coroutine>
#include <exception>
#include <iterator>
#include <type_traits>
#include <utility>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro {
    template <typename T>
    class ManualLifetime {
      public:
        ManualLifetime() noexcept {
            // We need to manually construct to avoid an attempted construction
        }

        ~ManualLifetime() {
            // We need this to get the union to destruct manually
        }

        template <typename... A>
        T &construct(A &&...args) noexcept(std::is_nothrow_constructible_v<T, A...>) {
            return *::new (static_cast<void *>(std::addressof(value))) T(static_cast<A &&>(args)...);
        }

        void destruct() noexcept(std::is_nothrow_destructible_v<T>) {
            value.~T();
        }

        T &get() & noexcept {
            return value;
        }
        T &&get() && noexcept {
            return static_cast<T &&>(value);
        }
        const T &get() const & noexcept {
            return value;
        }
        const T &&get() const && noexcept {
            return static_cast<const T &&>(value);
        }

      private:
        union {
            std::remove_const_t<T> value;
        };
    };

    template <typename T>
    class ManualLifetime<T &> {
      public:
        ManualLifetime() noexcept : value(nullptr) {
        }

        T &construct(T &value_in) noexcept {
            value = std::addressof(value_in);
            return value_in;
        }

        void destruct() noexcept {
            // No special destruction needed here
        }

        T &get() const noexcept {
            return *value;
        }

      private:
        T *value;
    };

    template <typename T>
    class ManualLifetime<T &&> {
      public:
        ManualLifetime() noexcept : value(nullptr) {
        }

        T &&construct(T &&value_in) noexcept {
            value = std::addressof(value_in);
            return static_cast<T &&>(value_in);
        }

        void destruct() noexcept {
            // No special destruction needed here
        }

        T &&get() const noexcept {
            return static_cast<T &&>(*value);
        }

      private:
        T *value;
    };

    struct UseAllocatorArg {};

    namespace ranges {

        RETROLIB_EXPORT template <typename R, typename A = UseAllocatorArg>
        struct ElementsOf {
            explicit constexpr ElementsOf(R &&range) noexcept
                requires std::is_default_constructible_v<A>
                : range(static_cast<R &&>(range)) {
            }

            constexpr ElementsOf(R &&range, A &&alloc) noexcept
                : range(static_cast<R &&>(range)), alloc(static_cast<A &&>(alloc)) {
            }

            constexpr ElementsOf(ElementsOf &&) noexcept = default;

            constexpr ElementsOf(const ElementsOf &) = delete;
            constexpr ElementsOf &operator=(const ElementsOf &) = delete;
            constexpr ElementsOf &operator=(ElementsOf &&) = delete;

            constexpr R &&get() noexcept {
                return static_cast<R &&>(range);
            }

            constexpr A get_allocator() const noexcept {
                return alloc;
            }

          private:
            [[no_unique_address]] A alloc; // \expos
            R &&range;                     // \expos
        };

        template <typename R>
        ElementsOf(R &&) -> ElementsOf<R>;

        template <typename R, typename A>
        ElementsOf(R &&, A &&) -> ElementsOf<R, A>;

    } // namespace ranges

    template <typename A>
    static constexpr bool allocator_needs_to_be_stored =
        !std::allocator_traits<A>::is_always_equal::value || !std::is_default_constructible_v<A>;

    // Round s up to next multiple of a.
    constexpr size_t aligned_allocation_size(size_t s, size_t a) {
        return (s + a - 1) & ~(a - 1);
    }

    RETROLIB_EXPORT template <typename R, typename V = std::remove_cvref_t<R>, typename A = UseAllocatorArg>
    class Generator;

    template <typename A>
    class PromiseBaseAlloc {
        static constexpr std::size_t offset_of_allocator(std::size_t frame_size) noexcept {
            return aligned_allocation_size(frame_size, alignof(A));
        }

        static constexpr std::size_t padded_frame_size(std::size_t frame_size) noexcept {
            return offset_of_allocator(frame_size) + sizeof(A);
        }

        static A &get_allocator(void *frame, std::size_t frame_size) noexcept {
            return *std::bit_cast<A *>(static_cast<char *>(frame) + offset_of_allocator(frame_size));
        }

      public:
        template <typename... T>
        static void *operator new(std::size_t frame_size, std::allocator_arg_t, A alloc, T &...) {
            void *frame = alloc.allocate(padded_frame_size(frame_size));

            // Store allocator at end of the coroutine frame.
            // Assuming the allocator's move constructor is non-throwing (a requirement for allocators)
            ::new (static_cast<void *>(std::addressof(get_allocator(frame, frame_size)))) A(std::move(alloc));

            return frame;
        }

        template <typename T, typename... U>
        static void *operator new(std::size_t frame_size, T &, std::allocator_arg_t, A alloc, U &...) {
            return PromiseBaseAlloc::operator new(frame_size, std::allocator_arg, std::move(alloc));
        }

        static void operator delete(void *ptr, std::size_t frame_size) noexcept {
            A &alloc = get_allocator(ptr, frame_size);
            A local_alloc(std::move(alloc));
            alloc.~A();
            local_alloc.deallocate(static_cast<std::byte *>(ptr), padded_frame_size(frame_size));
        }
    };

    template <typename A>
        requires(!allocator_needs_to_be_stored<A>)
    class PromiseBaseAlloc<A> {
      public:
        static void *operator new(std::size_t size) {
            A alloc;
            return alloc.allocate(size);
        }

        static void operator delete(void *ptr, std::size_t size) noexcept {
            A alloc;
            alloc.deallocate(static_cast<std::byte *>(ptr), size);
        }
    };

    template <typename T>
    struct GeneratorPromiseBase {
        template <typename R, typename V, typename A>
        friend class Generator;

        GeneratorPromiseBase *root;
        std::coroutine_handle<> parent_or_leaf;
        // Note: Using manual_lifetime here to avoid extra calls to exception_ptr
        // constructor/destructor in cases where it is not needed (i.e. where this
        // generator coroutine is not used as a nested coroutine).
        // This member is lazily constructed by the __yield_sequence_awaiter::await_suspend()
        // method if this generator is used as a nested generator.
        ManualLifetime<std::exception_ptr> exception;
        ManualLifetime<T> value;

        explicit GeneratorPromiseBase(std::coroutine_handle<> this_coro) noexcept
            : root(this), parent_or_leaf(this_coro) {
        }

        ~GeneratorPromiseBase() {
            if (root != this) {
                // This coroutine was used as a nested generator and so will
                // have constructed its __exception_ member which needs to be
                // destroyed here.
                exception.destruct();
            }
        }

        std::suspend_always initial_suspend() noexcept {
            return {};
        }

        void return_void() noexcept {
        }

        void unhandled_exception() {
            if (root != this) {
                exception.get() = std::current_exception();
            } else {
                throw;
            }
        }

        // Transfers control back to the parent of a nested coroutine
        struct FinalAwaiter {
            bool await_ready() noexcept {
                return false;
            }

            template <typename P>
            std::coroutine_handle<> await_suspend(std::coroutine_handle<P> handle) noexcept {
                auto &promise = handle.promise();
                auto &current_root = *promise.root;
                if (&current_root != &promise) {
                    auto parent = promise.parent_or_leaf;
                    current_root.parent_or_leaf = parent;
                    return parent;
                }
                return std::noop_coroutine();
            }

            void await_resume() noexcept {
                // No implementation needed here
            }
        };

        FinalAwaiter final_suspend() noexcept {
            return {};
        }

        std::suspend_always yield_value(T &&x) noexcept(std::is_nothrow_move_constructible_v<T>) {
            root->value.construct(std::move(x));
            return {};
        }

        template <typename V>
            requires(!std::is_reference_v<T>) && std::is_convertible_v<V, T>
        std::suspend_always yield_value(V &&x) noexcept(std::is_nothrow_constructible_v<T, V>) {
            root->value.construct(std::forward<V>(x));
            return {};
        }

        template <typename G>
        struct YieldSequenceAwaiter {
            G gen;

            explicit(false) YieldSequenceAwaiter(G &&gen) noexcept
                // Taking ownership of the generator ensures frame are destroyed
                // in the reverse order of their execution.
                : gen(static_cast<G &&>(gen)) {
            }

            bool await_ready() noexcept {
                return false;
            }

            // set the parent, root and exceptions pointer and
            // resume the nested
            template <typename Promise>
            std::coroutine_handle<> await_suspend(std::coroutine_handle<Promise> handle) noexcept {
                auto &current = handle.promise();
                auto &nested = *gen.get_promise();
                auto &current_root = *current.root;

                nested.root = current.root;
                nested.parent_or_leaf = handle;

                // Lazily construct the __exception_ member here now that we
                // know it will be used as a nested generator. This will be
                // destroyed by the promise destructor.
                nested.exception.construct();
                current_root.parent_or_leaf = gen.get_coro();

                // Immediately resume the nested coroutine (nested generator)
                return gen.get_coro();
            }

            void await_resume() {
                if (auto &nested_promise = *gen.get_promise(); nested_promise.exception.get()) {
                    std::rethrow_exception(std::move(nested_promise.exception.get()));
                }
            }
        };

        template <typename O, typename A>
        YieldSequenceAwaiter<Generator<T, O, A>>
        yield_value(ranges::ElementsOf<Generator<T, O, A>> generator) noexcept {
            return std::move(generator).get();
        }

        template <std::ranges::range R, typename A>
        YieldSequenceAwaiter<Generator<T, std::remove_cvref_t<T>, A>> yield_value(ranges::ElementsOf<R, A> &&x) {
            return [](std::allocator_arg_t, A alloc, auto &&range) -> Generator<T, std::remove_cvref_t<T>, A> {
                for (auto &&e : range)
                    co_yield static_cast<decltype(e)>(e);
            }(std::allocator_arg, x.get_allocator(), std::forward<R>(x.get()));
        }

        void resume() {
            parent_or_leaf.resume();
        }

        // Disable use of co_await within this coroutine.
        void await_transform() = delete;
    };

    template <typename G, typename B, bool ExplicitAllocator = false>
    struct GeneratorPromise;

    template <typename T, typename V, typename A, typename B, bool ExplicitAllocator>
    struct GeneratorPromise<Generator<T, V, A>, B, ExplicitAllocator> final : public GeneratorPromiseBase<T>,
                                                                              public PromiseBaseAlloc<B> {
        GeneratorPromise() noexcept
            : GeneratorPromiseBase<T>(std::coroutine_handle<GeneratorPromise>::from_promise(*this)) {
        }

        Generator<T, V, A> get_return_object() noexcept {
            return Generator<T, V, A>{std::coroutine_handle<GeneratorPromise>::from_promise(*this)};
        }

        using GeneratorPromiseBase<T>::yield_value;

        template <std::ranges::range R>
        typename GeneratorPromiseBase<T>::template YieldSequenceAwaiter<Generator<T, V, A>>
        yield_value(ranges::ElementsOf<R> &&x) {
            static_assert(!ExplicitAllocator, "This coroutine has an explicit allocator specified with "
                                              "std::allocator_arg so an allocator needs to be passed "
                                              "explicitly to std::elements_of");
            return [](auto &&range) -> Generator<T, V, A> {
                for (auto &&e : range)
                    co_yield std::forward<decltype(e)>(e);
            }(std::forward<R>(x.get()));
        }
    };

    template <typename A>
    using ByteAllocatorType = typename std::allocator_traits<std::remove_cvref_t<A>>::template rebind_alloc<std::byte>;
} // namespace retro

namespace std {
    // Type-erased allocator with default allocator behaviour.
    RETROLIB_EXPORT template <typename R, typename V, typename... A>
    struct coroutine_traits<retro::Generator<R, V>, A...> {
        using promise_type = retro::GeneratorPromise<retro::Generator<R, V>, std::allocator<std::byte>>;
    };

    // Type-erased allocator with std::allocator_arg parameter
    RETROLIB_EXPORT template <typename R, typename V, typename A, typename... T>
    struct coroutine_traits<retro::Generator<R, V>, allocator_arg_t, A, T...> {
      private:
        using ByteAllocator = retro::ByteAllocatorType<A>;

      public:
        using promise_type =
            retro::GeneratorPromise<retro::Generator<R, V>, ByteAllocator, true /*explicit Allocator*/>;
    };

    // Type-erased allocator with std::allocator_arg parameter (non-static member functions)
    RETROLIB_EXPORT template <typename R, typename V, typename S, typename A, typename... T>
    struct coroutine_traits<retro::Generator<R, V>, S, allocator_arg_t, A, T...> {
      private:
        using ByteAllocator = retro::ByteAllocatorType<A>;

      public:
        using promise_type =
            retro::GeneratorPromise<retro::Generator<R, V>, ByteAllocator, true /*explicit Allocator*/>;
    };

    // Generator with specified allocator type
    RETROLIB_EXPORT template <typename R, typename V, typename A, typename... T>
    struct coroutine_traits<retro::Generator<R, V, A>, T...> {
        using ByteAllocator = retro::ByteAllocatorType<A>;

      public:
        using promise_type = retro::GeneratorPromise<retro::Generator<R, V, A>, ByteAllocator>;
    };
} // namespace std

namespace retro {
    /**
     * @class Generator
     * @brief The Generator class is a type of coroutine powered view. Allowing the user to define a method that
     * executes until a `co_yield` statement and then stalling until the next iteration of the loop.
     *
     * @tparam R The reference type of the generator
     * @tparam V The value type of the generator (defaults to the cvref unqualified version of R)
     * @tparam A The allocator used for this type
     */
    template <typename R, typename V, typename A>
    class Generator : public std::ranges::view_interface<Generator<R, V, A>> {
        using ByteAllocator = ByteAllocatorType<A>;

      public:
        using promise_type = GeneratorPromise<Generator, ByteAllocator>;
        friend promise_type;

      private:
        using CoroutineHandle = std::coroutine_handle<promise_type>;

      public:
        Generator() noexcept = default;

        Generator(Generator &&other) noexcept
            : coroutine(std::exchange(other.coroutine, {})), started(std::exchange(other.started, false)) {
        }

        ~Generator() noexcept {
            if (coroutine) {
                if (started && !coroutine.done()) {
                    coroutine.promise().value.destruct();
                }
                coroutine.destroy();
            }
        }

        Generator &operator=(Generator &&g) noexcept {
            swap(g);
            return *this;
        }

        void swap(Generator &other) noexcept {
            std::swap(coroutine, other.coroutine);
            std::swap(started, other.started);
        }

        struct Sentinel {};

        class Iterator {
          public:
            using iterator_category = std::input_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = V;
            using reference = R;
            using pointer = std::add_pointer_t<R>;

            Iterator() noexcept = default;
            Iterator(const Iterator &) = delete;

            Iterator(Iterator &&other) noexcept : coroutine(std::exchange(other.coroutine, {})) {
            }

            Iterator &operator=(const Iterator &other) = delete;

            Iterator &operator=(Iterator &&other) noexcept {
                std::swap(coroutine, other.coroutine);
                return *this;
            }

            ~Iterator() = default;

            friend bool operator==(const Iterator &it, Sentinel) noexcept {
                return it.coroutine.done();
            }

            Iterator &operator++() {
                coroutine.promise().value.destruct();
                coroutine.promise().resume();
                return *this;
            }
            void operator++(int) {
                (void)operator++();
            }

            reference operator*() const noexcept {
                return static_cast<reference>(coroutine.promise().value.get());
            }

          private:
            friend Generator;

            explicit Iterator(CoroutineHandle coroutine) noexcept : coroutine(coroutine) {
            }

            CoroutineHandle coroutine;
        };

        Iterator begin() {
            RETROLIB_ASSERT(coroutine);
            RETROLIB_ASSERT(!started);
            started = true;
            coroutine.resume();
            return Iterator{coroutine};
        }

        Sentinel end() noexcept {
            return {};
        }

      private:
        explicit Generator(CoroutineHandle coroutine) noexcept : coroutine(coroutine) {
        }

      public: // to get around access restrictions for __yield_sequence_awaitable
        std::coroutine_handle<> get_coro() noexcept {
            return coroutine;
        }
        promise_type *get_promise() noexcept {
            return std::addressof(coroutine.promise());
        }

      private:
        CoroutineHandle coroutine;
        bool started = false;
    };

    template <typename R, typename V>
    class Generator<R, V, UseAllocatorArg> : public std::ranges::view_interface<Generator<R, V>> {
        using PromiseBase = GeneratorPromiseBase<R>;

      public:
        Generator() noexcept = default;

        Generator(Generator &&other) noexcept
            : promise(std::exchange(other.promise, nullptr)), coroutine(std::exchange(other.coroutine, {})),
              started(std::exchange(other.started, false)) {
        }

        ~Generator() noexcept {
            if (coroutine) {
                if (started && !coroutine.done()) {
                    promise->value.destruct();
                }
                coroutine.destroy();
            }
        }

        Generator &operator=(Generator g) noexcept {
            swap(g);
            return *this;
        }

        void swap(Generator &other) noexcept {
            std::swap(promise, other.promise);
            std::swap(coroutine, other.coroutine);
            std::swap(started, other.started);
        }

        struct Sentinel {};

        class Iterator {
          public:
            using iterator_category = std::input_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = V;
            using reference = R;
            using pointer = std::add_pointer_t<R>;

            Iterator() noexcept = default;
            Iterator(const Iterator &) = delete;

            Iterator(Iterator &&other) noexcept
                : promise(std::exchange(other.promise, nullptr)), coroutine(std::exchange(other.coroutine, {})) {
            }

            Iterator &operator=(const Iterator &) = default;

            Iterator &operator=(Iterator &&other) noexcept {
                promise = std::exchange(other.promise, nullptr);
                coroutine = std::exchange(other.coroutine, {});
                return *this;
            }

            ~Iterator() = default;

            friend bool operator==(const Iterator &it, Sentinel) noexcept {
                return it.coroutine.done();
            }

            Iterator &operator++() {
                promise->value.destruct();
                promise->resume();
                return *this;
            }

            void operator++(int) {
                (void)operator++();
            }

            reference operator*() const noexcept {
                return static_cast<reference>(promise->value.get());
            }

          private:
            friend Generator;

            explicit Iterator(PromiseBase *promise, std::coroutine_handle<> coroutine) noexcept
                : promise(promise), coroutine(coroutine) {
            }

            PromiseBase *promise;
            std::coroutine_handle<> coroutine;
        };

        Iterator begin() {
            RETROLIB_ASSERT(coroutine);
            RETROLIB_ASSERT(!started);
            started = true;
            coroutine.resume();
            return Iterator{promise, coroutine};
        }

        Sentinel end() noexcept {
            return {};
        }

      private:
        template <typename G, typename B, bool ExplicitAllocator>
        friend struct GeneratorPromise;

        template <typename P>
        explicit Generator(std::coroutine_handle<P> coroutine) noexcept
            : promise(std::addressof(coroutine.promise())), coroutine(coroutine) {
        }

      public: // to get around access restrictions for __yield_sequence_awaitable
        std::coroutine_handle<> get_coro() noexcept {
            return coroutine;
        }
        PromiseBase *get_promise() noexcept {
            return promise;
        }

      private:
        PromiseBase *promise = nullptr;
        std::coroutine_handle<> coroutine;
        bool started = false;
    };
} // namespace retro