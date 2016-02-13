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

#include "backtrace/demangle.h"

#ifdef __GNUC__
#  include <cstdlib>
#  include <cxxabi.h>
#elif defined(_MSC_VER)
#  include <windows.h>
#  include <Dbghelp.h>
#endif

namespace bt {

namespace {

#if defined(__GNUC__)
inline string demangle_impl(const string& function_name) {
  if (function_name.size() == 0) {
    return "";
  }
  int status;
  char *real_name = abi::__cxa_demangle(function_name.c_str(),
                                        NULL,
                                        NULL,
                                        &status);
  string result;
  if (real_name != NULL) {
    result = real_name;
    free(real_name);
  } else {
    result = function_name + "()";
  }
  return result;
}
#elif defined(_MSC_VER)
inline string demangle_impl(const string& function_name) {
  string demangled_name;
  demangled_name.reserve(function_name.size() * 2);
  UnDecorateSymbolName(function_name.c_str(),
                       &demangled_name[0],
                       (DWORD)demangled_name.capacity(),
                       UNDNAME_COMPLETE);
  return function_name + "()";
}
#else
inline string demangle_impl(const string& function_name) {
  return function_name;
}
#endif

}  // namespace

string demangle(const string& function_name) {
  return demangle_impl(function_name);
}

}  // namespace bt
