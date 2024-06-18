#pragma once

#include <utility>

template <typename TupleT, size_t... Is>
inline void tuple_map_impl(const TupleT& tp, std::index_sequence<Is...>, auto const& f) {
  (f(std::get<Is>(tp), Is), ...);
}

template <typename TupleT, size_t TupSize = std::tuple_size_v<TupleT>>
inline void tuple_map(const TupleT& tp, auto const& f) {
  tuple_map_impl(tp, std::make_index_sequence<TupSize>{}, f);
}
