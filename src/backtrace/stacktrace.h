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

#ifndef __STACKTRACE_H__
#define __STACKTRACE_H__

#include "backtrace/backtrace_util.h"

namespace bt {

// Entry in the stack trace.
struct TraceEntry {
  // Frame address corresponding to the stack entry.
  void *address;

  TraceEntry()
  : address(NULL) { }

  explicit TraceEntry(void *address)
  : address(address) { }
};

// Generic stack trace gatherer, defines common interface only,
// requires actual implementation based on whatever libraries are
// available.
//
// This class is basically a general wrapper for a vector which
// contains frames information. For the purpose of data acquisition
// speed is left to the actual implementation internals, and it only
// gets converted to a public API when operator[] is used. This way
// it's possible to collect stack trace information from a loops
// and then symbolize them once it's really needed.
class StackTrace {
 public:
  // Create an actual stack trace implementation.
  static StackTrace *create();

  // Get current stack address.
  static void *current_addr_get(void *context);

  // Default constructor.
  StackTrace() {}

  // Default destructor.
  virtual ~StackTrace() {}

  // Load stack trace starting from a given address.
  // Return size of the stack loaded.
  virtual size_t load(void *addr, size_t depth = 64) = 0;

  // Get number of entries in the trace.
  virtual size_t size() const = 0;

  // Get stack trace entry with a given index.
  virtual TraceEntry operator[](size_t index) const = 0;
};

namespace internal {

StackTrace *stacktrace_create_stub();

#ifdef BACKTRACE_HAS_CAPTURE_STACK_BACKTRACE
StackTrace *stacktrace_create_capture_stack_backtrace();
#endif

#ifdef BACKTRACE_HAS_EXECINFO
StackTrace *stacktrace_create_execinfo();
#endif

}  // namespace internal

}  // namespace bt

#endif  // __STACKTRACE_H__
