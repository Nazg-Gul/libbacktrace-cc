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

#ifndef __BACKTRACE_UTIL_H__
#define __BACKTRACE_UTIL_H__

#include <cassert>
#include <map>
#include <string>
#include <sstream>
#include <vector>

namespace bt {

using std::string;
using std::vector;
using std::map;

template<typename T>
inline T hex_cast(const string& in) {
  T out;
  std::stringstream ss;
  ss << in;
  ss >> std::hex >> out;
  return out;
}

template<typename T>
inline string hex_cast(const T in) {
  std::stringstream ss;
  ss << std::hex << in;
  return "0x" + ss.str();
}

}  // namespace bt

// Check whether execinfo.h is available.
#if defined(__linux__) || defined(__APPLE__)
#  define BACKTRACE_HAS_EXECINFO
#endif

// Convert build system's defines to a in-project ones.
#ifdef WITH_BFD
#  define BACKTRACE_HAS_BFD
#endif
#ifdef WITH_UCONTEXT
#  define BACKTRACE_HAS_UCONTEXT
#endif

#endif  /* __BACKTRACE_UTIL_H__ */
