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

#include <limits.h>
#include <stdlib.h>
#include <bfd.h>
#include <unistd.h>

#include "backtrace/demangle.h"

#ifdef BACKTRACE_HAS_BFD

namespace bt {
namespace internal {

namespace {

// Sybolize implementation using BFD library.
class SymbolizeBfd : public Symbolize {
 public:
  // Initialize BFD library.
  static bool init() {
    if (bfd_ != NULL) {
      return true;
    }

    // Get the path to an executable file.
    char exe_name[PATH_MAX];
    int res = readlink("/proc/self/exe", exe_name, sizeof(exe_name));
    if (res == -1) {
        return false;
    }
    exe_name[res] = 0;

    // Initalize BFD and open the executable file.
    bfd_init();
    bfd_ = bfd_openr(exe_name, 0);
    if (bfd_ == NULL) {
        return false;
    }

    if (!bfd_check_format(bfd_, bfd_object)) {
      return false;
    }
    if ((bfd_get_file_flags(bfd_) & HAS_SYMS) == 0) {
      return false;
    }
    unsigned storage_needed = bfd_get_symtab_upper_bound(bfd_);
    asymbols_ = (asymbol **)malloc(storage_needed);
    if (bfd_canonicalize_symtab(bfd_, asymbols_) <= 0) {
      return false;
    }
    text_ = bfd_get_section_by_name(bfd_, ".text");
    return true;
  }

  SymbolizeBfd() : Symbolize() {
  }

  explicit SymbolizeBfd(StackTrace *stack_trace)
      : Symbolize(stack_trace) {
    if (stack_trace_ != NULL) {
      resolve(*stack_trace_);
    }
  }

  void resolve(const StackTrace& stacktrace) {
    if (text_ == NULL) {
      // Happens if BFD initialization fails.
      symbols_.resize(0);
      return;
    }
    symbols_.resize(stacktrace.size());
    for (size_t i = 0; i < stacktrace.size(); ++i) {
      void *address = (void*)((unsigned char*)(stacktrace[i].address) - 1);
      resolve(address, &symbols_[i]);
    }
  }

 private:
  static bfd *bfd_;
  static asymbol **asymbols_;
  static asection *text_;

  // Resolve given address into a symbol description.
  void resolve(void *address,
               Symbol *symbol) {
    bfd_vma offset = ((bfd_vma)address) - text_->vma;
    if (offset > 0) {
      const char *file_name;
      const char *function_name;
      unsigned line_number;
      if (bfd_find_nearest_line(bfd_,
                                text_,
                                asymbols_,
                                offset,
                                &file_name,
                                &function_name,
                                &line_number)) {
        symbol->address = (size_t)address;
        if (file_name != NULL) {
          symbol->file_name = file_name;
          if (line_number != 0) {
            symbol->line_number = line_number;
          } else {
            symbol->line_number = Symbol::LINE_NONE;
          }
        }
        if (function_name != NULL) {
          symbol->function_name = demangle(function_name);
        }
      }
    }
  }
};

bfd *SymbolizeBfd::bfd_ = NULL;
asymbol **SymbolizeBfd::asymbols_ = NULL;
asection *SymbolizeBfd::text_ = NULL;

}  // namespace

Symbolize *symbolize_create_bfd(StackTrace *stack_trace) {
  if (!SymbolizeBfd::init()) {
    return NULL;
  }
  return new SymbolizeBfd(stack_trace);
}

}  // namespace internal
}  // namespace bt

#endif  // BACKTRACE_HAS_BFD
