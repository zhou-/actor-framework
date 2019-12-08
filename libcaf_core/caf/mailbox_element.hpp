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

#include <cstddef>
#include <memory>

#include "caf/actor_control_block.hpp"
#include "caf/config.hpp"
#include "caf/detail/core_export.hpp"
#include "caf/detail/tuple_vals.hpp"
#include "caf/detail/type_erased_tuple_view.hpp"
#include "caf/extend.hpp"
#include "caf/intrusive/singly_linked.hpp"
#include "caf/make_message.hpp"
#include "caf/message.hpp"
#include "caf/message_id.hpp"
#include "caf/meta/omittable_if_empty.hpp"
#include "caf/meta/type_name.hpp"
#include "caf/ref_counted.hpp"
#include "caf/tracing_data.hpp"
#include "caf/type_erased_tuple.hpp"

namespace caf {

class CAF_CORE_EXPORT mailbox_element
  : public intrusive::singly_linked<mailbox_element> {
public:
  using forwarding_stack = std::vector<strong_actor_ptr>;

  /// Content of this message.
  message content;

  /// Source of this message and receiver of the final response.
  strong_actor_ptr sender;

  /// Denotes whether this an asynchronous message or a request.
  message_id mid;

  /// `stages.back()` is the next actor in the forwarding chain,
  /// if this is empty then the original sender receives the response.
  forwarding_stack stages;

#ifdef CAF_ENABLE_ACTOR_PROFILER
  /// Optional tracing information. This field is unused by default, but an
  /// @ref actor_profiler can make use of it to inject application-specific
  /// instrumentation.
  tracing_data_ptr tracing_id;
#endif // CAF_ENABLE_ACTOR_PROFILER

  mailbox_element(message content, strong_actor_ptr sender, message_id mid,
                  forwarding_stack stages);

  bool is_high_priority() const {
    return mid.category() == message_id::urgent_message_category;
  }

  mailbox_element() = delete;

  mailbox_element(const mailbox_element&) = delete;

  mailbox_element& operator=(const mailbox_element&) = delete;
};

/// @relates mailbox_element
template <class Inspector>
auto inspect(Inspector& f, mailbox_element& x) {
  return f(meta::type_name("mailbox_element"), x.content, x.sender, x.mid,
           meta::omittable_if_empty(), x.stages
#ifdef CAF_ENABLE_ACTOR_PROFILER
           ,
           x.tracing_id
#endif // CAF_ENABLE_ACTOR_PROFILER
  );
}

/// @relates mailbox_element
using mailbox_element_ptr = std::unique_ptr<mailbox_element>;

/// @relates mailbox_element
CAF_CORE_EXPORT mailbox_element_ptr
make_mailbox_element(strong_actor_ptr sender, message_id id,
                     mailbox_element::forwarding_stack stages, message msg);

/// @relates mailbox_element
template <class T, class... Ts>
std::enable_if_t<!std::is_same<typename std::decay<T>::type, message>::value
                   || (sizeof...(Ts) > 0),
                 mailbox_element_ptr>
make_mailbox_element(strong_actor_ptr sender, message_id id,
                     mailbox_element::forwarding_stack stages, T&& x,
                     Ts&&... xs) {
  return std::make_unique<mailbox_element>(
    make_message(std::forward<T>(x), std::forward<Ts>(xs)...),
    std::move(sender), id, std::move(stages));
}

} // namespace caf
