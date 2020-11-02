// Copyright (C) 2020  Valentin B.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

/**
 * @file defines.h
 * @brief Broadly useful language definitions for Aurora.
 * @author Valentin B.
 */

#ifndef AURORA_COMMON_DEFINES_H_
#define AURORA_COMMON_DEFINES_H_

#if defined(_MSC_VER)
#define AURORA_ALWAYS_INLINE __forceinline
#define AURORA_NEVER_INLINE __declspec(noinline)
#define AURORA_NORETURN __declspec(noreturn)
#elif defined(__GNUC__)
#define AURORA_ALWAYS_INLINE inline __attribute__((always_inline))
#define AURORA_NEVER_INLINE __attribute__((noinline))
#define AURORA_NORETURN __attribute__((noreturn))
#else
#define AURORA_ALWAYS_INLINE inline
#define AURORA_NEVER_INLINE
#define AURORA_NORETURN
#endif

// AURORA_CURRENT_FILE_NAME provides the name of the file in which this macro
// was invoked.
#define AURORA_CURRENT_FILE_NAME __FILE__

// AURORA_CURRENT_LINE_NUMBER provides the line number where this macro was
// invoked.
#define AURORA_CURRENT_LINE_NUMBER __LINE__

// AURORA_CURRENT_FUNCTION stores the function name or the entire function
// signature, if possible, and can be used to dynamically retrieve this
// information in functions.
#if defined(__PRETTY_FUNCTION__)
#define AURORA_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
#define AURORA_CURRENT_FUNCTION __FUNCSIG__
#else
#define AURORA_CURRENT_FUNCTION __FUNCTION__
#endif

// Declares a class to be non-copyable.
#define AURORA_DISALLOW_COPY(_TYPE) _TYPE(const _TYPE&) = delete

// Declares a class to be non-assignable.
#define AURORA_DISALLOW_ASSIGN(_TYPE) _TYPE& operator=(const _TYPE&) = delete

// Declares a class to be non-copyable and non-assignable.
#define AURORA_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  AURORA_DISALLOW_COPY(TypeName);                 \
  AURORA_DISALLOW_ASSIGN(TypeName)

// Declares a class to be non-movable.
#define AURORA_DISALLOW_MOVE(_TYPE) \
  _TYPE(_TYPE&&) = delete;          \
  _TYPE& operator=(_TYPE&&) = delete

// AURORA_PREDICT can be used to increase code performance where the outcome of
// an expression is predictable with a high probability. Overusing this macro
// will not increase code performance and is considered bad practice.
#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define AURORA_PREDICT(_EXPR, _RES, _PROBABILITY)                        \
  __builtin_expect_with_probability(_EXPR, _RES, ({                      \
                                      constexpr double probability =     \
                                          _PROBABILITY;                  \
                                      static_assert(0.0 <= probability); \
                                      static_assert(probability <= 1.0); \
                                      probability;                       \
                                    }))
#else
#define AURORA_PREDICT(_EXPR, _RES, _PROBABILITY) ((_RES) == !!(_EXPR))
#endif

// AURORA_PREDICT_TRUE and AURORA_PREDICT_FALSE can improve the performance of
// conditional evaluations where the result is highly likely to turn out as
// predicted through the probability. Overusing these macros will not increase
// code performance and is considered bad practice.
#define AURORA_PREDICT_TRUE(_EXPR, _PROBABILITY) \
  AURORA_PREDICT(!!(_EXPR), 1, _PROBABILITY)
#define AURORA_PREDICT_FALSE(_EXPR, _PROBABILITY) \
  AURORA_PREDICT(!!(_EXPR), 0, _PROBABILITY)

// AURORA_LIKELY and AURORA_UNLIKELY increase the performance of conditionals
// where the result has a very high probability to turn out as either true xor
// false. Overusing these macros will not increase code performance and is
// considered bad practice.
#define AURORA_LIKELY(_EXPR) AURORA_PREDICT_TRUE(_EXPR, 1.0)
#define AURORA_UNLIKELY(_EXPR) AURORA_PREDICT_FALSE(_EXPR, 1.0)

namespace aurora {
namespace common {

/**
 * @brief A helper function to discard the given arguments in optimizer output.
 * @tparam ArgTypes The types of the supplied arguments.
 * @param args The arguments.
 */
template <typename... ArgTypes>
constexpr AURORA_ALWAYS_INLINE void unused_impl(ArgTypes&&... args) {
  (static_cast<void>(args), ...);
}

}  // namespace common
}  // namespace aurora

// Discards the supplied arguments in case the code does not need them.
#define AURORA_UNUSED(...) ::aurora::common::unused_impl(__VA_ARGS__)

#endif  // AURORA_COMMON_DEFINES_H_
