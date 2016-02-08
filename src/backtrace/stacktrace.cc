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

#include "backtrace/stacktrace.h"

#ifdef BACKTRACE_HAS_UCONTEXT
#  include <ucontext.h>
#endif

namespace bt {

StackTrace *StackTrace::create() {
#ifdef BACKTRACE_HAS_EXECINFO
  return internal::stacktrace_create_execinfo();
#else
  return internal::stacktrace_create_stub();
#endif
}

void *StackTrace::current_addr_get(void *context) {
#ifdef BACKTRACE_HAS_UCONTEXT
  ucontext_t *ucontext = (ucontext_t *)context;
  if (ucontext != NULL) {
#if defined(REG_RIP)  // x86_64
    return (void *)ucontext->uc_mcontext.gregs[REG_RIP];
#elif defined(REG_EIP)  // xi686
    return (void *)ucontext->uc_mcontext.gregs[REG_EIP];
#else
#  warning "No clue how to get program counter  from a context"
#endif
  }
#endif  // BACKTRACE_HAS_UCONTEXT
  return NULL;
}

}  // namespace bt
