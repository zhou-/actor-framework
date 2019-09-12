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

#pragma once

#include <typeinfo>

#include "caf/atom.hpp"
#include "caf/detail/type_traits.hpp"
#include "caf/span.hpp"
#include "caf/type_nr.hpp"
#include "caf/variant.hpp"

namespace caf {

/// Runtime-type information for actor communication.
class messaging_interface {
public:
  // -- member types -----------------------------------------------------------

  /// Runtime-type token for patterns. Either a non-zero type number for
  /// builtin types, or a type information pointer to user-defined types, or a
  /// CAF atom.
  struct token {
    /// Denotes which value in the union is valid.
    uint8_t tag;

    union {
      /// Stores the type number for builtin types.
      uint16_t type_nr;

      /// Points to the C++ RTTI object for custom types.
      const std::type_info* rtti;

      /// Stores the value of atom constants.
      atom_value value;
    };

    constexpr explicit token(uint16_t type_nr) noexcept
      : tag(0), type_nr(type_nr) {
      // nop
    }

    constexpr explicit token(const std::type_info* rtti) noexcept
      : tag(1), rtti(rtti) {
      // nop
    }

    constexpr explicit token(atom_value value) noexcept : tag(2), value(value) {
      // nop
    }
  };

  /// Creates a token for builtin CAF types.
  template <class T>
  static constexpr detail::enable_if_t<
    type_nr<T>::value != 0 && !is_atom_constant<T>::value, token>
  make_token() noexcept {
    return token{type_nr<T>::value};
  }

  /// Creates a token for custom types.
  template <class T>
  static constexpr detail::enable_if_t<type_nr<T>::value == 0, token>
  make_token() noexcept {
    return token{&typeid(T)};
  }

  /// Creates a token for CAF atoms.
  template <class T>
  static constexpr detail::enable_if_t<is_atom_constant<T>::value, token>
  make_token() noexcept {
    return token{T::get_value()};
  }

  /// Bundles in- and ouptut types for a single communiation pattern.
  struct pattern {
    /// Lists input types for the pattern.
    span<const token> inputs;

    /// Lists output types for the pattern.
    span<const token> outputs;
  };

  /// Denotes the type of a messaging interface.
  enum class type {
    /// Denotes that the interface of an actor is unknown, because the actor is
    /// not running locally.
    unknown,
    /// Denotes that the actor is dynamically typed.
    dynamically_typed,
    /// Denotes that the actor is statically typed and this messaging interface
    /// maps accepted inputs to outputs.
    statically_typed,
  };

  /// Tag type for constructing messaging interfaces for dynamically typed
  /// actors.
  struct dynamically_typed_tag {};

  // -- constructors, destructors, and assignment operators --------------------

  /// Creates an invalid interface.
  messaging_interface();

  /// Creates a dynamically typed messaging interface.
  explicit messaging_interface(dynamically_typed_tag);

  /// Creates a statically typed messaging interface.
  explicit messaging_interface(span<const pattern> patterns);

  messaging_interface(const messaging_interface&) = default;

  messaging_interface& operator=(const messaging_interface&) = default;

  // -- properties -------------------------------------------------------------

  /// Returns whether this messaging interface is valid.
  explicit operator bool() const noexcept {
    return type_ != type::unknown;
  }

  /// Returns whether this messaging interface is invalid
  /// (default-constructed).
  bool operator!() const noexcept {
    return type_ == type::unknown;
  }

  /// Returns whether this messaging interface belongs to a dynamically typed
  /// actor.
  bool dynamically_typed() const noexcept {
    return type_ == type::dynamically_typed;
  }

  /// Returns whether this messaging interface belongs to a statically typed
  /// actor.
  bool statically_typed() const noexcept {
    return type_ == type::statically_typed;
  }

private:
  ///  Stores which type of actor we are dealing with.
  type type_;

  /// Stores in- and output types for statically-typed actors.
  span<const pattern> patterns_;
};

} // namespace caf
