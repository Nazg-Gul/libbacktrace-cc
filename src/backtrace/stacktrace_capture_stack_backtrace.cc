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

#ifdef BACKTRACE_HAS_CAPTURE_STACK_BACKTRACE

#include <windows.h>
#include <limits.h>

namespace bt {
namespace internal {

namespace {

// Stack trace implementation using CaptureStackBackTrace().
class StackTraceCaptureStackBacktrace : public StackTrace {
 public:
  StackTraceCaptureStackBacktrace() : StackTrace() {}

  size_t load(void * /*addr*/, size_t depth) {
    backtrace_buffer_.resize(depth);
    DWORD num_frames = CaptureStackBackTrace(0,
                                             (DWORD)depth,
                                             &backtrace_buffer_[0],
                                             NULL);
    backtrace_buffer_.resize(num_frames);
    // TODO(sergey): Move backtrace_buffer_ to addr if it's not NULL.
    return num_frames;
  }

  size_t size() const {
    return backtrace_buffer_.size();
  }

  TraceEntry operator[](size_t index) const {
    assert(index < size());
    TraceEntry entry(backtrace_buffer_[index]);
    return entry;
  }
 private:
  vector<void*> backtrace_buffer_;
};

}  // namespace

StackTrace *stacktrace_create_capture_stack_backtrace() {
  return new StackTraceCaptureStackBacktrace();
}

}  // namespace internal
}  // namespace bt

#endif  // BACKTRACE_HAS_CAPTURE_STACK_BACKTRACE
