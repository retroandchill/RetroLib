/**
 * @file Concat.h
 * @brief TODO: Fill me out
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/Ranges/RangeBasics.h"
#include "RetroLib/Ranges/Concepts/Concatable.h"
#include "RetroLib/Utils/Unreachable.h"

#include <variant>
#include <tuple>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro::ranges {

    RETROLIB_EXPORT template <std::ranges::input_range... R>
        requires (std::ranges::view<R> && ...) && (sizeof...(R) > 0) && Concatable<R...>
    class ConcatView : public std::ranges::view_interface<ConcatView<R...>> {
        using DifferenceType = std::common_type_t<std::ranges::range_difference_t<R>...>;
        static constexpr size_t ranges_size = sizeof...(R);
        std::tuple<R...> ranges;

        template <bool IsConst>
        struct Iterator;

        template <bool IsConst>
        struct Sentinel {
        private:
            friend struct Sentinel<!IsConst>;
            friend struct Iterator<IsConst>;
            template <typename T>
            using ConstifyIf = std::conditional_t<IsConst, const T, T>;
            using ConcatViewType = ConstifyIf<ConcatView>;
            std::ranges::sentinel_t<ConstifyIf<LastInPack<R...>>> end_element;

        public:
            constexpr Sentinel() = default;
            explicit constexpr Sentinel(ConcatViewType& view, EndTag) : end_element(std::ranges::end(std::get<ranges_size - 1>(view.ranges))) {}

            template <bool Other>
                requires IsConst && (!Other)
            explicit constexpr Sentinel(Sentinel<Other> other) : end_element(std::move(other.end_element)) {

            }
        };

        template <bool IsConst>
        struct Iterator {
            using difference_type = std::common_type_t<std::ranges::range_difference_t<R>...>;
            using value_type = ConcatValue<R...>;

        private:
            friend struct Iterator<!IsConst>;
            template <typename T>
            using ConstifyIf = std::conditional_t<IsConst, const T, T>;
            using ConcatViewType = ConstifyIf<ConcatView>;
            ConcatView* view;
            std::variant<std::ranges::iterator_t<ConstifyIf<R>>...> it;



            template <size_t N>
            constexpr void satisfy() {
                RETROLIB_ASSERT(it.index() == N);
                if constexpr (N < ranges_size - 1) {
                    if (std::get<N>(view->ranges) == std::ranges::end(std::get<N>(view->ranges))) {
                        it.template emplace<N + 1>(std::ranges::begin(std::get<N + 1>(view->ranges)));
                        satisfy<N + 1>();
                    }
                }
            }

            struct Next {
                Iterator* pos;
                template <std::input_iterator I, size_t N>
                constexpr void operator()(IndexedElement<I, N> it) const {
                    RETROLIB_ASSERT(it.get() != std::ranges::end(std::get<N>(pos->view->ranges)));
                   ++it.get();
                    pos->satisfy<N>();
                }
            };

            struct Prev {
                Iterator *pos;

                template <std::bidirectional_iterator I>
                constexpr void operator()(IndexedElement<I, 0> it) const {
                    RETROLIB_ASSERT(it.get() != std::ranges::begin(std::get<0>(pos->view->ranges)));
                    --it.get();
                }

                template <std::bidirectional_iterator I, size_t N>
                    requires (N != 0)
                constexpr void operator()(IndexedElement<I, N> it) const {
                    if (it.get() != std::ranges::begin(std::get<N>(pos->view->ranges))) {
                        auto&& rng = std::get<N - 1>(pos->view->ranges);
                        pos->it.template emplace<N - 1>(std::ranges::next(std::ranges::begin(rng), std::ranges::end(rng)));
                        std::visit(*this, pos->it);
                    } else {
                        --it.get();
                    }
                }
            };

            struct AdvanceForward {
                Iterator* pos;
                difference_type n;

                template <std::random_access_iterator I>
                constexpr void operator()(IndexedElement<I, ranges_size - 1> it) const {
                    std::ranges::advance(it.get(), n);
                }

                template <std::random_access_iterator I, size_t N>
                constexpr void operator()(IndexedElement<I, N> it) const {
                    auto last = std::ranges::end(std::get<N>(pos->view->ranges));
                    auto rest = std::ranges::advance(it.get(), n, std::move(last));
                    pos->satisfy<N>();

                    if (rest != 0) {
                        std::visit(AdvanceForward{pos, rest}, pos->it);
                    }
                }
            };

            struct AdvanceReverse {
                Iterator* pos;
                difference_type n;

                template <std::random_access_iterator I>
                constexpr void operator()(IndexedElement<I, 0> it) const {
                    std::ranges::advance(it.get(), n);
                }

                template <std::random_access_iterator I, std::size_t N>
                constexpr void operator()(IndexedElement<I, N> it) const {
                    auto first = std::ranges::begin(std::get<N>(pos->view->ranges));
                    if (it.get() == first) {
                        auto &&rng = std::get<N - 1>(pos->view->ranges);
                        pos->it.template emplace<N - 1>(std::ranges::next(std::ranges::begin(rng), std::ranges::end(rng)));
                        std::visit(*this, pos->it);
                    } else {
                        auto rest = std::ranges::advance(it.get(), n, std::move(first));
                        if (rest != 0) {
                            std::visit(AdvanceReverse{pos, rest}, pos->it);
                        }
                    }
                }
            };

            [[noreturn]] static difference_type distance_to(std::integral_constant<size_t, ranges_size>) {
                RETROLIB_ASSERT(false);
                unreachable();
            }

            template <size_t N>
            static difference_type distance_to(std::integral_constant<size_t, N>, const Iterator& from, const Iterator& to) {
                if (from.it.index() > N) {
                    return Iterator::distance_to(std::integral_constant<size_t, N + 1>{}, from, to);
                }

                if (from.it.index() == N) {
                    if (to.it.index() == N) {
                        return std::ranges::distance(std::get<N>(from.it), std::get<N>(to.it));
                    }

                    return std::ranges::distance(std::get<N>(from.it), std::ranges::end(std::get<N>(from.view->ranges))) +
                        Iterator::distance_to(std::integral_constant<size_t, N + 1>{}, from, to);
                }

                if (from.it.index() < N && to.it.index() > N) {
                    return std::ranges::distance(std::get<N>(from.view->ranges)) +
                        Iterator::distance_to(std::integral_constant<size_t, N + 1>{}, from, to);
                }

                RETROLIB_ASSERT(to.it.index() == N);
                return std::ranges::distance(std::get<N>(from.it), std::get<N>(to.it));
            }

        public:
            using Reference = std::common_reference_t<std::ranges::range_reference_t<ConstifyIf<R>>...>;
            static constexpr bool single_pass = (SinglePassIterator<std::ranges::iterator_t<R>> || ...);

            constexpr Iterator() = default;
            constexpr Iterator(ConcatViewType& view, BeginTag) : view(&view), it(std::in_place_index<0>, std::get<0>(view.ranges)) {
                satisfy<0>();
            }

            constexpr Iterator(ConcatViewType& view, EndTag) : view(&view), it(std::in_place_index<ranges_size - 1>, std::get<ranges_size - 1>(view.ranges)) {}

            template <bool Other>
                requires IsConst && (!Other)
            explicit constexpr Iterator(Iterator<Other> other) : view(other.view), it(std::move(other.it)) {}

            constexpr Reference operator*() const {
                return std::visit<Reference, []<typename T> (T&& it) { return *std::forward<T>(it); }, it>;
            }

            constexpr Iterator& operator++() {
                std::visit(Next{this}, it);
                return *this;
            }

            constexpr auto operator++(int) {
                if constexpr ((std::ranges::forward_range<R> && ...)) {
                    auto tmp = *this;
                    ++*this;
                    return tmp;
                } else {
                    ++*this;
                }
            }

            constexpr bool operator==(const Iterator& pos) const requires std::equality_comparable<std::variant<std::ranges::iterator_t<ConstifyIf<R>>...>> {
                return it == pos.it;
            }

            constexpr bool operator==(const Sentinel<IsConst>& post) const {
                return it.index() == ranges_size - 1 && std::get<ranges_size - 1>(view->it) == post.end_element;
            }

            constexpr std::partial_ordering operator<=>(const Iterator& other) const requires (std::random_access_iterator<std::ranges::iterator_t<R>> && ...) {
                auto distance = *this - other;
                if (distance == 0) {
                    return std::partial_ordering::equivalent;
                }

                if (distance < 0) {
                    return std::partial_ordering::less;
                }

                return std::partial_ordering::greater;
            }

            constexpr Iterator& operator--() requires (std::bidirectional_iterator<std::ranges::iterator_t<R>> && ...) {
                std::visit(Prev{this}, it);
                return *this;
            }

            constexpr auto operator--(int) {
                if constexpr ((std::ranges::forward_range<R> && ...)) {
                    auto tmp = *this;
                    --*this;
                    return tmp;
                } else {
                    --*this;
                }
            }

            constexpr Iterator operator+(difference_type n) const requires (std::random_access_iterator<std::ranges::iterator_t<R>> && ...) {
                Iterator tmp(*this);
                tmp += n;
                return tmp;
            }

            friend Iterator operator+(difference_type n, const Iterator& it) requires (std::random_access_iterator<std::ranges::iterator_t<R>> && ...) {
                return it + n;
            }

            constexpr Iterator& operator+=(difference_type n) requires (std::random_access_iterator<std::ranges::iterator_t<R>> && ...) {
                if (n > 0) {
                    std::visit(AdvanceForward{this, n}, it);
                } else if (n < 0) {
                    std::visit(AdvanceReverse{this, -n}, it);
                }
                return *this;
            }

            constexpr Iterator operator-(difference_type n) const requires (std::random_access_iterator<std::ranges::iterator_t<R>> && ...) {
                Iterator tmp(*this);
                tmp -= n;
                return tmp;
            }

            constexpr difference_type operator-(const Iterator& other) const requires (std::sized_sentinel_for<std::ranges::iterator_t<R>, std::ranges::iterator_t<R>> && ...) {
                return distance_to(std::integral_constant<size_t, ranges_size - 1>{}, *this, other);
            }

            constexpr Iterator& operator-=(difference_type n) requires (std::random_access_iterator<std::ranges::iterator_t<R>> && ...) {
                return *this += -n;
            }

            constexpr Reference operator[](difference_type n) const requires (std::random_access_iterator<std::ranges::iterator_t<R>> && ...) {
                return *(*this + n);
            }
        };

    public:
        constexpr ConcatView() = default;

        constexpr explicit ConcatView(R... ranges) : ranges(std::move(ranges)...) {}

        constexpr auto begin() {
            return Iterator<(SimpleView<R> && ...)>(*this, BeginTag{});
        }

        constexpr Iterator<true> begin() const requires (std::ranges::range<const R> && ...) {
            return Iterator<true>(*this, BeginTag{});
        }

        constexpr auto end() {
            if constexpr ((std::ranges::common_range<R> && ...)) {
                return Iterator<(SimpleView<R> && ...)>(*this, EndTag{});
            } else {
                return Sentinel<(SimpleView<R> && ...)>(*this, EndTag{});
            }
        }

        constexpr auto end() const requires (std::ranges::range<const R> && ...) {
            if constexpr ((std::ranges::common_range<R> && ...)) {
                return Iterator<true>(*this, EndTag{});
            } else {
                return Sentinel<true>(*this, EndTag{});
            }
        }

        constexpr size_t size() const requires (std::ranges::sized_range<R> && ...) {
            return std::apply([](auto&... r) { return (std::ranges::size(r) + ...); }, ranges);
        }
    };

    template<typename... Rng>
    ConcatView(Rng &&...) -> ConcatView<std::ranges::views::all_t<Rng>...>;


    namespace views {
        struct ConcatInvoker {

            template <std::ranges::input_range... R>
                requires std::constructible_from<ConcatView<R...>, R...> && (sizeof...(R) > 0) && Concatable<R...>
            constexpr auto operator()(R&&... ranges) const {
                return ConcatView(std::forward<R>(ranges)...);
            }
        };

        RETROLIB_EXPORT constexpr ConcatInvoker concat;
    }
}
