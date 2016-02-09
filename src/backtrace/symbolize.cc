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

namespace bt {

Symbolize *Symbolize::create(StackTrace *stacktrace) {
#if defined(BACKTRACE_HAS_BFD)
  return internal::symbolize_create_bfd(stacktrace);
#elif defined(BACKTRACE_HAS_EXECINFO)
  return internal::symbolize_create_execinfo(stacktrace);
#elif defined(BACKTRACE_HAS_SYM_FROM_ADDR)
  return internal::symbolize_create_sym_from_addr(stacktrace);
#else
  return internal::symbolize_create_stub(stacktrace);
#endif
}

Symbolize::~Symbolize() {
  if (stacktrace_ != NULL) {
    delete stacktrace_;
  }
}

}  // namespace bt
