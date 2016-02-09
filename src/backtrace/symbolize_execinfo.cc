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

#include "backtrace/symbolize.h"

#ifdef BACKTRACE_HAS_EXECINFO

#include <cstdlib>
#include <execinfo.h>

#include "backtrace/demangle.h"

namespace bt {
namespace internal {

namespace {

// Sybolize implementation using execinfo header and backtrace() call.
class SymbolizeExecinfo : public Symbolize {
 public:
  SymbolizeExecinfo() : Symbolize() {}
  explicit SymbolizeExecinfo(StackTrace *stacktrace)
      : Symbolize(stacktrace) {
    if (stacktrace_ != NULL) {
      resolve(*stacktrace_);
    }
  }

  void resolve(const StackTrace& stacktrace) {
    // Create buffer which backtrace() understand from an alien stacktrace.
    vector<void*> backtrace_buffer(stacktrace.size(), NULL);
    for (size_t i = 0; i < stacktrace.size(); ++i) {
      TraceEntry entry = stacktrace[i];
      backtrace_buffer[i] = entry.address;
    }
    // Gather symbols information and convert them to Symbol.
    char **strings = backtrace_symbols(&backtrace_buffer[0],
                                       stacktrace.size());
    symbols_.resize(backtrace_buffer.size());
    for (size_t i = 0; i < backtrace_buffer.size(); ++i) {
      parse(strings[i], &symbols_[i]);
    }

    free(strings);
  }

 private:
  // Parse string returned by backtrace_symbols() and pyt results
  // to a given Symbol.
  void parse(const string& str,
             Symbol *symbol) {
    // Get frame address.
    size_t addr_start = str.find_last_of('[');
    assert(addr_start != string::npos);
    symbol->address = hex_cast<size_t>(
            str.substr(addr_start + 1,
                       str.size() - addr_start - 2));
    // Get function name and offset.
    size_t function_start = str.find_last_of('(');
    assert(addr_start != string::npos);
    string function_and_offset =
            str.substr(function_start + 1, addr_start - function_start - 3);
    if (function_and_offset.size()) {
      size_t plus = function_and_offset.find_last_of('+');
      assert(plus != string::npos);
      symbol->function_name = demangle(function_and_offset.substr(0, plus));
      symbol->function_offset = hex_cast<size_t>(
              function_and_offset.substr(plus + 1,
                      function_and_offset.size() - plus - 1));
    } else {
      symbol->function_name = "";
      symbol->function_offset = 0x00;
    }
    // Get object name.
    symbol->object_name = str.substr(0, function_start);
  }
};

}  // namespace

Symbolize *symbolize_create_execinfo(StackTrace *stacktrace) {
  return new SymbolizeExecinfo(stacktrace);
}

}  // namespace internal
}  // namespace bt

#endif  // BACKTRACE_HAS_EXECINFO
