// Copyright (C) 2016 libbacktrace-cc authors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
// Author: sergey@blender.org (Sergey Sharybin)

#include "backtrace/backtrace.h"

#include <iomanip>
#include <sstream>

#include "backtrace/stacktrace.h"
#include "backtrace/symbolize.h"

namespace bt {

namespace {

string backtrace_get_string() {
  StackTrace *stacktrace = StackTrace::create();
  stacktrace->load(NULL, BACKTRACE_MAX_DEPTH);
  Symbolize *symbolize = Symbolize::create(stacktrace);

  std::stringstream ss;
  for (size_t i = 0; i < symbolize->size(); ++i) {
    const Symbol& symbol = symbolize->at(i);
    // Frame index.
    ss << std::right << std::setw(8) << i;
    // TODO(sergey): Show object name in some nice format,
    // which doesn't screw alignment up.
    // Frame address.
    ss << "  " << std::right << std::setw(16)
       << ((symbol.address != Symbol::ADDRESS_NONE)
                   ? hex_cast(symbol.address)
                   : "(nil)");
    // Function name.
    string function_name = ((symbol.function_name.size() > 0)
                                   ? symbol.function_name
                                   : "(unknown)");
    ss << "    " << function_name;
    // Source file or function offset.
    if (symbol.file_name.size() == 0) {
      if (symbol.function_offset != Symbol::OFFSET_NONE) {
        ss << "+" << hex_cast(symbol.function_offset);
      }
    } else {
      const int N = 8;
      size_t pad = ((size_t)((function_name.size() + N - 1) / N)) * N
                   - function_name.size();
      ss << std::setfill(' ') << std::setw(pad) << ""
         << " " << symbol.file_name;
      if (symbol.line_number != Symbol::LINE_NONE) {
         ss << ":" << symbol.line_number;
      }
    }
    ss << "\n";
  }
  delete symbolize;
  return ss.str();
}

void backtrace_print(FILE *fp) {
  string backtrace = backtrace_get_string();
  fputs(backtrace.c_str(), fp);
}

}  // namespace

}  // namespace bt

void backtrace_print(FILE *fp) {
  bt::backtrace_print(fp);
}
