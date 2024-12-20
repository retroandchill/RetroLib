/**
 * @file JoinWith.h
 * @brief Allows the user to join with another view in-between.
 *
 * @author Retro & Chill
 * https://github.com/retroandchill
 */
#pragma once

#if !RETROLIB_WITH_MODULES
#include "RetroLib/Ranges/Concepts/Concatable.h"
#include "RetroLib/Ranges/RangeBasics.h"

#include <variant>
#endif

#ifndef RETROLIB_EXPORT
#define RETROLIB_EXPORT
#endif

namespace retro::ranges {

    template <typename I>
    struct StoreInner {
        NonPropagatingCache<std::remove_cv_t<I>> inner;

        template <typename O>
        constexpr auto&& update_inner(O&& it) {
            return inner.emplace_deref(it);
        }

        constexpr I& get_inner(auto) {
            return *inner;
        }
    };

    struct PassThroughInner {
        template <typename O>
        static constexpr auto&& update_inner(O&& it) noexcept(noexcept(*std::forward<O>(it))) {
            return *std::forward<O>(it);
        }

        template <typename O>
        static constexpr decltype(auto) get_inner(O&& it) {
            return *std::forward<O>(it);
        }
    };

    template <std::ranges::input_range R>
    using JoinViewInner = std::conditional_t<!std::is_reference_v<std::ranges::range_reference_t<R>>,
        StoreInner<std::ranges::range_reference_t<R>>, PassThroughInner>;

    RETROLIB_EXPORT template <std::ranges::input_range R, std::ranges::forward_range P>
        requires std::ranges::view<R> && std::ranges::input_range<std::ranges::range_reference_t<R>> && std::ranges::view<P> && Concatable<std::ranges::range_reference_t<R>, P>
    class JoinWithView : public std::ranges::view_interface<JoinWithView<R, P>>, private JoinViewInner<R> {
        using Outer = std::ranges::views::all_t<R>;
        using Inner = std::ranges::views::all_t<std::ranges::range_reference_t<R>>;
        using SizeType = std::common_type_t<std::ranges::range_difference_t<Inner>, std::ranges::range_difference_t<P>>;

        class Iterator {

        public:
            using value_type = std::common_type_t<std::ranges::range_value_t<Inner>, std::ranges::range_value_t<P>>;
            using reference = std::common_reference_t<std::ranges::range_reference_t<Inner>, std::ranges::range_reference_t<P>>;
            using rvalue_reference = std::common_reference_t<std::ranges::range_rvalue_reference_t<Inner>, std::ranges::range_rvalue_reference_t<P>>;
            using single_pass = std::true_type;
            using difference_type = SizeType;

            constexpr Iterator() = default;
            constexpr explicit Iterator(JoinWithView& range) : range(&range), outer_it(std::ranges::begin(range.outer)) {
                if (outer_it != std::ranges::end(range.outer)) {
                    auto &&inner = range.update_inner(outer_it);
                    current.template emplace<1>(std::ranges::begin(inner));
                    satisfy();
                }
            }

            constexpr bool operator==(std::default_sentinel_t) const {
                return outer_it == std::ranges::end(range->outer);
            }

            constexpr Iterator& operator++() {
                if (current.index() == 0) {
                    auto &it = std::get<0>(current);
                    RETROLIB_ASSERT(it != std::ranges::end(range->contraction));
                    ++it;
                } else {
                    auto &it = std::get<1>(current);
#ifndef NDEBUG
                    auto &&inner = range->get_inner(outer_it);
                    RETROLIB_ASSERT(it != std::ranges::end(inner));
#endif
                    ++it;
                }
                satisfy();
                return *this;
            }

            constexpr void operator++(int) {
                ++*this;
            }

            constexpr reference operator*() const & {
                if (current.index() == 0) {
                    return *std::get<0>(current);
                }

                return *std::get<1>(current);
            }

            constexpr rvalue_reference operator*() && {
                if (current.index() == 0) {
                    return std::move(*std::get<0>(current));
                }

                return std::move(*std::get<1>(current));
            }

        private:
            void satisfy() {
                while (true) {
                    if (current.index() == 0) {
                        if (std::get<0>(current) != std::ranges::end(range->contraction)) {
                            break;
                        }

                        auto &&inner = range->update_inner(outer_it);
                        current.template emplace<1>(std::ranges::begin(inner));
                    } else {
                        auto &&inner = range->get_inner(outer_it);
                        if (std::get<1>(current) != std::ranges::end(inner)) {
                            break;
                        }

                        ++outer_it;
                        if (outer_it == std::ranges::end(range->outer)) {
                            break;
                        }

                        current.template emplace<0>(std::ranges::begin(range->contraction));
                    }
                }
            }

            JoinWithView* range = nullptr;
            std::ranges::iterator_t<R> outer_it;
            std::variant<std::ranges::iterator_t<P>, std::ranges::iterator_t<Inner>> current;
        };

    public:
        constexpr JoinWithView() = default;
        constexpr JoinWithView(R outer, P contraction)  : outer(std::move(outer)), contraction(std::move(contraction)) { }

        constexpr Iterator begin() {
            return Iterator(*this);
        }

        constexpr std::default_sentinel_t end() {
            return std::default_sentinel;
        }


    private:

        Outer outer;
        std::ranges::views::all_t<P> contraction;

    };

    template <std::ranges::input_range R, std::ranges::forward_range P>
    JoinWithView(R&&, P&&) -> JoinWithView<std::ranges::views::all_t<R>, std::ranges::views::all_t<P>>;

    namespace views {
        RETROLIB_EXPORT template <std::ranges::input_range R, std::ranges::forward_range P>
                requires std::ranges::viewable_range<R> && std::ranges::input_range<std::ranges::range_reference_t<R>> && std::ranges::viewable_range<P> && Concatable<std::ranges::range_reference_t<R>, P>
        constexpr auto join_with(R&& range, P&& contraction) {
            return JoinWithView(std::ranges::views::all(std::forward<R>(range)), std::ranges::views::all(std::forward<P>(contraction)));
        }

        RETROLIB_EXPORT template <std::ranges::input_range R, typename P>
        requires std::ranges::viewable_range<R> && std::ranges::input_range<std::ranges::range_reference_t<R>> &&
            std::convertible_to<P, std::ranges::range_value_t<std::ranges::range_reference_t<R>>>
        constexpr auto join_with(R&& range, P&& contraction) {
            return join_with(std::forward<R>(range), std::ranges::views::single(std::forward<P>(contraction)));
        }

        struct JoinWithInvoker {

            template <std::ranges::input_range R, std::ranges::forward_range P>
                requires std::ranges::viewable_range<R> && std::ranges::input_range<std::ranges::range_reference_t<R>> && std::ranges::viewable_range<P> && Concatable<std::ranges::range_reference_t<R>, P>
            constexpr auto operator()(R&& range, P&& contraction) const {
                return join_with(std::forward<R>(range), std::forward<P>(contraction));
            }

            template <std::ranges::input_range R, typename P>
                requires std::ranges::viewable_range<R> && std::ranges::input_range<std::ranges::range_reference_t<R>> &&
                    std::convertible_to<P, std::ranges::range_value_t<std::ranges::range_reference_t<R>>>
            constexpr auto operator()(R&& range, P&& contraction) const {
                return join_with(std::forward<R>(range), std::forward<P>(contraction));
            }
        };

        RETROLIB_EXPORT constexpr JoinWithInvoker join_with_invoker;

        RETROLIB_EXPORT template <typename P>
        constexpr auto join_with(P&& contraction) {
            return extension_method<join_with_invoker>(std::forward<P>(contraction));
        }

    }

}