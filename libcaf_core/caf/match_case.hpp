/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright 2011-2018 Dominik Charousset                                     *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#pragma once

#include <tuple>
#include <type_traits>

#include "caf/detail/apply_args.hpp"
#include "caf/detail/core_export.hpp"
#include "caf/detail/int_list.hpp"
#include "caf/detail/invoke_result_visitor.hpp"
#include "caf/detail/pseudo_tuple.hpp"
#include "caf/detail/try_match.hpp"
#include "caf/detail/type_list.hpp"
#include "caf/detail/type_traits.hpp"
#include "caf/message.hpp"
#include "caf/none.hpp"
#include "caf/optional.hpp"
#include "caf/param.hpp"
#include "caf/skip.hpp"

namespace caf {

class CAF_CORE_EXPORT match_case {
public:
  enum result { no_match, match, skip };

  explicit match_case(uint32_t tt);

  match_case(const match_case&) = delete;

  match_case& operator=(const match_case&) = delete;

  virtual ~match_case();

  /// Tries to invoke this match case with the contents of `xs`.
  virtual result
  invoke(detail::invoke_result_visitor& rv, type_erased_tuple& xs)
    = 0;

  uint32_t type_token() const noexcept {
    return token_;
  }

private:
  uint32_t token_;
};

template <class F>
class trivial_match_case : public match_case {
public:
  using fun_trait = typename detail::get_callable_trait<F>::type;

  using plain_result_type = typename fun_trait::result_type;

  using result_type = typename std::conditional<
    std::is_reference<plain_result_type>::value, plain_result_type,
    typename std::remove_const<plain_result_type>::type>::type;

  using arg_types = typename fun_trait::arg_types;

  static constexpr bool is_manipulator
    = detail::tl_exists<arg_types, detail::is_mutable_ref>::value;

  using pattern = typename detail::tl_map<arg_types, param_decay>::type;

  using decayed_arg_types =
    typename detail::tl_map<arg_types, std::decay>::type;

  using intermediate_pseudo_tuple
    = detail::tl_apply_t<decayed_arg_types, detail::pseudo_tuple>;

  trivial_match_case(const trivial_match_case&) = delete;

  trivial_match_case& operator=(const trivial_match_case&) = delete;

  explicit trivial_match_case(F f)
    : match_case(make_type_token_from_list<pattern>()), fun_(std::move(f)) {
    // nop
  }

  match_case::result
  invoke(detail::invoke_result_visitor& f, type_erased_tuple& xs) {
    detail::meta_elements<pattern> ms;
    // check if try_match() reports success
    if (!detail::try_match(xs, ms.arr.data(), ms.arr.size()))
      return match_case::no_match;
    typename detail::il_indices<decayed_arg_types>::type indices;
    message tmp;
    auto needs_detaching = is_manipulator && xs.shared();
    if (needs_detaching)
      tmp = message::copy(xs);
    intermediate_pseudo_tuple tup{needs_detaching ? tmp.content() : xs};
    if constexpr (std::is_same<result_type, void>::value) {
      apply_args(fun_, indices, tup);
      auto fun_res = unit;
      return f.visit(fun_res) ? match_case::match : match_case::skip;
    } else {
      auto fun_res = apply_args(fun_, indices, tup);
      return f.visit(fun_res) ? match_case::match : match_case::skip;
    }
  }

private:
  F fun_;
};

struct match_case_info {
  uint32_t type_token;
  match_case* ptr;
};

inline bool operator<(const match_case_info& x, const match_case_info& y) {
  return x.type_token < y.type_token;
}

} // namespace caf
