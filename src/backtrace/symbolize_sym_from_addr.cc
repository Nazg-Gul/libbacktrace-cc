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

#ifdef BACKTRACE_HAS_SYM_FROM_ADDR

// Maximum symbol length.
#define MAX_SYMBOL 256

#include <windows.h>
#include <dbghelp.h>

// TODO(sergey): Consider moving this to CMakeLists.
#pragma comment(lib, "dbghelp.lib")

namespace bt {
namespace internal {

namespace {

// Sybolize implementation using SymFromAddr().
class SymbolizeSymFromAddr : public Symbolize {
 public:
  SymbolizeSymFromAddr() : Symbolize() {}
  explicit SymbolizeSymFromAddr(StackTrace *stacktrace)
      : Symbolize(stacktrace) {
    if (stacktrace_ != NULL) {
      resolve(*stacktrace_);
    }
  }

  void resolve(const StackTrace& stacktrace) {
    // Make sure symbol table is initialized.
    init_sym();
    // Allocate some working memory.
    const size_t total_size = sizeof(SYMBOL_INFO) + MAX_SYMBOL;
    SYMBOL_INFO *symbol_info;
    symbol_info = reinterpret_cast<SYMBOL_INFO *>(::operator new(total_size));
    symbol_info->MaxNameLen = MAX_SYMBOL - 1;
    symbol_info->SizeOfStruct = sizeof(SYMBOL_INFO);
    // Symbolize stacktrace.
    HANDLE process = GetCurrentProcess();
    symbols_.resize(stacktrace.size());
    for (size_t i = 0; i < stacktrace.size(); ++i) {
      symbols_[i].address = (size_t)stacktrace[i].address;
      if (SymFromAddr(process,
                      (DWORD64)(stacktrace[i].address),
                      0,
                      symbol_info)) {
        symbols_[i].address = symbol_info->Address;
        symbols_[i].function_name = symbol_info->Name;
      }
    }
    delete symbol_info;
  }
 private:
  void init_sym() {
    static bool sym_initialized = false;
    if (!sym_initialized) {
      HANDLE process = GetCurrentProcess();
      SymInitialize(process, NULL, TRUE);
    }
  }
};

}  // namespace

Symbolize *symbolize_create_sym_from_addr(StackTrace *stacktrace) {
  return new SymbolizeSymFromAddr(stacktrace);
}

}  // namespace internal
}  // namespace bt

#undef MAX_SYMBOL

#endif  // BACKTRACE_HAS_SYM_FROM_ADDR
