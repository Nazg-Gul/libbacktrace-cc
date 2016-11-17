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

#ifndef __SYMBOLIZE_H__
#define __SYMBOLIZE_H__

#include "backtrace/backtrace_util.h"
#include "backtrace/stacktrace.h"

#if (__cplusplus > 199711L) || (defined(_MSC_VER) && _MSC_VER >= 1800)
#  define BT_SCOPED_ENUM(name, type) enum name : type
#else
#  define BT_SCOPED_ENUM(name, type) enum name
#endif

namespace bt {

// Human-readable symbol information.
struct Symbol {
  // TODO(sergey): This is an int for MSVC, need different approach.
  BT_SCOPED_ENUM(LineConstant, signed int) {
    LINE_NONE = -1,
  };

  BT_SCOPED_ENUM(OffsetConstants, size_t) {
    ADDRESS_NONE = ~((size_t)0),
    OFFSET_NONE = ~((size_t)0),
  };

  string object_name;
  size_t address;
  string file_name;
  int line_number;
  string function_name;
  size_t function_offset;

  Symbol()
  : object_name(""),
    address(ADDRESS_NONE),
    file_name(""),
    line_number(LINE_NONE),
    function_name(),
    function_offset(OFFSET_NONE) {}

  Symbol(const string& object_name,
         size_t address,
         const string& file_name,
         int line_number,
         const string& function_name,
         size_t function_offset)
  : object_name(object_name),
    address(address),
    file_name(file_name),
    line_number(line_number),
    function_name(function_name),
    function_offset(function_offset) {}
};

class Symbolize {
 public:
  // Create an actual symbolizer implementation.
  static Symbolize *create(StackTrace *stacktrace = NULL);

  // Default constructor.
  Symbolize()
  : stacktrace_(NULL) {}

  explicit Symbolize(StackTrace *stacktrace)
  : stacktrace_(stacktrace) {}

  // Default constructor.
  virtual ~Symbolize();

  // Get total number of symbols.
  virtual size_t size() { return symbols_.size(); }

  // Get symbol at given stack index.
  virtual inline const Symbol& at(size_t index) const {
    assert(index < symbols_.size());
    return symbols_[index];
  }

  // Get symbol at given stack index.
  virtual inline const Symbol& operator[](size_t index) const {
    return at(index);
  }

  virtual void resolve(const StackTrace& stacktrace) = 0;

 protected:
  StackTrace *stacktrace_;
  vector<Symbol> symbols_;
};

namespace internal {

Symbolize *symbolize_create_stub(StackTrace *stacktrace = NULL);

#ifdef BACKTRACE_HAS_EXECINFO
Symbolize *symbolize_create_execinfo(StackTrace *stacktrace = NULL);
#endif

#ifdef BACKTRACE_HAS_BFD
Symbolize *symbolize_create_bfd(StackTrace *stacktrace = NULL);
#endif

#ifdef BACKTRACE_HAS_SYM_FROM_ADDR
Symbolize *symbolize_create_sym_from_addr(StackTrace *stacktrace = NULL);
#endif

}  // namespace internal

}  // namespace bt

#undef BT_SCOPED_ENUM

#endif  // __SYMBOLIZE_H__
