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

#ifdef BACKTRACE_HAS_STACK_WALK

#include <windows.h>
#include <imagehlp.h>

namespace bt {
namespace internal {

namespace {

// Stack trace implementation using StackWalk64().
class StackTraceStackWalk : public StackTrace {
 public:
  StackTraceStackWalk() : StackTrace() {}

  size_t load(void * /*addr*/, size_t depth) {
    init_symbol_handler();
    // Some initialization.
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();
    //  Context, we start with the current address here.
    CONTEXT context;
    RtlCaptureContext(&context);
    // Stack frame descriptor.
    STACKFRAME64 stack = init_stack_frame(context);
    DWORD machine_type = machine_type_get();
    // Actual stack tracing.
    backtrace_buffer_.resize(depth);
    size_t num_frames = 0;
    do {
      if (!StackWalk64(machine_type,
                       process,
                       thread,
                       &stack,
                       &context,
                       NULL,
                       SymFunctionTableAccess64,
                       SymGetModuleBase64,
                       NULL)) {
        // TODO(sergey): Clear stack trace?
        break;
      }
      backtrace_buffer_[num_frames] =
              reinterpret_cast<void *>(stack.AddrPC.Offset);
      ++num_frames;
    } while (stack.AddrReturn.Offset != 0);
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
  STACKFRAME64 init_stack_frame(const CONTEXT& c) {
    STACKFRAME64 s = {0};
#ifdef _M_X64
    s.AddrPC.Offset = c.Rip;
    s.AddrStack.Offset = c.Rsp;
    s.AddrFrame.Offset = c.Rbp;
#else
    s.AddrPC.Offset = c.Eip;
    s.AddrStack.Offset = c.Esp;
    s.AddrFrame.Offset = c.Ebp;
#endif
    s.AddrPC.Mode = AddrModeFlat;
    s.AddrStack.Mode = AddrModeFlat;
    s.AddrFrame.Mode = AddrModeFlat;
    return s;
  }

  HMODULE current_module_get() {
    DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS;
    HMODULE module = 0;
    GetModuleHandleEx(flags,
                      reinterpret_cast<LPCTSTR>(stacktrace_create_stack_walk),
                      &module);
    return module;
  }

  DWORD machine_type_get() {
    HMODULE module = current_module_get();
    IMAGE_NT_HEADERS *headers = ImageNtHeader(module);
    DWORD image_type = headers->FileHeader.Machine;
    return image_type;
  }

  vector<void*> backtrace_buffer_;
};

}  // namespace

StackTrace *stacktrace_create_stack_walk() {
  return new StackTraceStackWalk();
}

}  // namespace internal
}  // namespace bt

#endif  // BACKTRACE_HAS_STACK_WALK
