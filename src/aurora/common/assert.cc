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

#include "common/assert.h"

#include <cstdlib>
#include <iostream>
#include <sstream>

// TODO: Implement proper logging.

namespace aurora {
namespace common {

template <typename Arg>
void write_args(std::ostream &os, Arg arg) {
  os << arg;
}

template <typename... Args>
AURORA_NORETURN void assert_fail_impl(const char *file,
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
