/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright (C) 2011 - 2017                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#include "caf/policy/broadcast.hpp"

#include "caf/abstract_downstream.hpp"

namespace caf {
namespace policy {

long broadcast::total_net_credit(const abstract_downstream& out) {
  // The buffer on `out` is shared on all paths. Our total available number of
  // net credit is thus calculates as `av_min + mb - bs`, where `av_min` is the
  // minimum of available credits on all paths, `mb` is the minimum buffer
  // size, and `bs` is the current buffer size.
  return out.min_credit() + out.min_buffer_size() - out.buf_size();
}

void broadcast::push(abstract_downstream& out, long* hint) {
  out.broadcast(hint);
}

} // namespace policy
} // namespace caf