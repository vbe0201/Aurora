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
 * @file assert.h
 * @brief Common assertion helpers to be used by Aurora.
 * @author Valentin B.
 */

#ifndef AURORA_COMMON_ASSERT_H_
#define AURORA_COMMON_ASSERT_H_

#include <cstdlib>
#include <iostream>
#include <sstream>

#include "defines.h"

// TODO: Implement proper logging.

namespace aurora {
namespace common {

template <typename Arg>
void write_args(std::ostream &os, Arg arg) {
  os << arg;
}

/**
 * @brief A helper function that is called when an assertion in Aurora fails.
 * @tparam Args The type of variadic arguments for variable log messages.
 * @param file The file in which the assertion failed.
 * @param line The line at which the assertion happened.
 * @param func The function in which the assertion took place.
 * @param expr The expression that was asserted.
 * @param args Variable arguments for custom and more verbose log messages.
 * @note The function is defined in this header file because templated functions
 * are only generated when they are actually needed.
 */
template <typename... Args>
AURORA_NORETURN AURORA_NEVER_INLINE void assert_fail_impl(const char *file,
                                                          int line,
                                                          const char *func,
                                                          const char *expr,
                                                          Args... args) {
  // Write all variadic arguments to the string stream.
  std::ostringstream oss;
  write_args(oss, args...);

  std::cout << "Assertion Failure: " << oss.str() << "\n";
  std::cout << "Location:   " << file << ":" << line << "\n";
  std::cout << "Function:   " << func << "\n";
  std::cout << "Expression: " << expr << "\n";

  std::abort();
}

}  // namespace common
}  // namespace aurora

// AURORA_CALL_ASSERT_FAIL_IMPL calls the aurora::common::assert_fail_impl
// function with a given condition and the variable arguments for custom
// log messages. The other arguments for the call are provided by this macro.
#define AURORA_CALL_ASSERT_FAIL_IMPL(_COND, ...)            \
  ::aurora::common::assert_fail_impl(                       \
      AURORA_CURRENT_FILE_NAME, AURORA_CURRENT_LINE_NUMBER, \
      AURORA_CURRENT_FUNCTION, _COND, ##__VA_ARGS__)

// AURORA_ASSERT_IMPL takes an expression and asserts it. If the assertion
// fails, execution will be aborted and a custom set of arguments providing
// details for debugging the issue will be logged to the default stdout.
#define AURORA_ASSERT_IMPL(_EXPR, ...)                        \
  {                                                           \
    if (const bool __assert_value = static_cast<bool>(_EXPR); \
        AURORA_UNLIKELY(__assert_value)) {                    \
      AURORA_CALL_ASSERT_FAIL_IMPL(#_EXPR, ##__VA_ARGS__);    \
    }                                                         \
  }

// Evaluates the supplied expression and compares it against an expected result
// and if there's no match, the program will be terminated in an error.
#define AURORA_ASSERT(_EXPR, ...) AURORA_ASSERT_IMPL(_EXPR, ##__VA_ARGS__)

// An unreachable branch in the program's control flow. If the program ever encounters
// this, execution will be halted and a message
#define AURORA_UNREACHABLE() AURORA_ASSERT_IMPL(true, "entered unreachable code")

#endif  // AURORA_COMMON_ASSERT_H_
