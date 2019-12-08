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

#include "caf/mailbox_element.hpp"

#include "caf/message_builder.hpp"
#include "caf/type_nr.hpp"

namespace caf {

namespace {

message_id dynamic_category_correction(const message& msg, message_id x) {
  auto tt = msg.type_token();
  if (tt == make_type_token<downstream_msg>())
    return x.with_category(message_id::downstream_message_category);
  if (tt == make_type_token<upstream_msg>())
    return x.with_category(message_id::upstream_message_category);
  return x;
}

} // namespace

mailbox_element::mailbox_element(message msg, strong_actor_ptr sender,
                                 message_id mid, forwarding_stack stages)
  : content(std::move(msg)),
    sender(std::move(sender)),
    mid(dynamic_category_correction(content, mid)),
    stages(std::move(stages)) {
  // nop
}

mailbox_element_ptr
make_mailbox_element(strong_actor_ptr sender, message_id id,
                     mailbox_element::forwarding_stack stages, message msg) {
  return std::make_unique<mailbox_element>(std::move(msg), std::move(sender),
                                           id, std::move(stages));
}

} // namespace caf
