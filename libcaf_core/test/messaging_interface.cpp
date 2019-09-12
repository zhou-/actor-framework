/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright 2011-2019 Dominik Charousset                                     *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#define CAF_SUITE messaging_interface

#include "caf/messaging_interface.hpp"

#include "caf/test/dsl.hpp"

#include "caf/atom.hpp"

using namespace caf;

namespace {

struct fixture {};

} // namespace

CAF_TEST_FIXTURE_SCOPE(messaging_interface_tests, fixture)

CAF_TEST(tokens) {
  auto tk1 = messaging_interface::make_token<actor>();
  CAF_CHECK_EQUAL(tk1.tag, 0u);
  CAF_CHECK_EQUAL(tk1.type_nr, 1u);
  auto tk2 = messaging_interface::make_token<fixture>();
  CAF_CHECK_EQUAL(tk2.tag, 1u);
  CAF_CHECK_EQUAL(tk2.rtti, &typeid(fixture));
  auto tk3 = messaging_interface::make_token<ok_atom>();
  CAF_CHECK_EQUAL(tk3.tag, 2u);
  CAF_CHECK_EQUAL(tk3.value, atom("ok"));
}

CAF_TEST_FIXTURE_SCOPE_END()
