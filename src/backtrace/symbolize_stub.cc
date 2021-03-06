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
namespace internal {

namespace {

// Stub sybolize implementation.
class SymbolizeStub : public Symbolize {
 public:
  SymbolizeStub() : Symbolize() {}
  explicit SymbolizeStub(StackTrace *stacktrace)
      : Symbolize(stacktrace) {
    if (stacktrace_ != NULL) {
      resolve(*stacktrace_);
    }
  }

  void resolve(const StackTrace& stacktrace) {
    symbols_.resize(stacktrace.size());
    for (size_t i = 0; i < stacktrace.size(); ++i) {
      symbols_[i].address = (size_t)stacktrace[i].address;
    }
  }
};

}  // namespace

Symbolize *symbolize_create_stub(StackTrace *stacktrace) {
  return new SymbolizeStub(stacktrace);
}

}  // namespace internal
}  // namespace bt
