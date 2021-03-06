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

#ifdef BACKTRACE_HAS_BFD

#include <bfd.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

#include "backtrace/demangle.h"

#define GNU_DEBUGLINK ".gnu_debuglink"

namespace bt {
namespace internal {

namespace {

class BfdSymbols {
 public:
  BfdSymbols()
      : bfd_(NULL),
        symtab_(NULL),
        dynamic_symtab_(NULL),
        text_(NULL),
        debug_link_(NULL) {
    init(self_object_name_get());
  }

  explicit BfdSymbols(const string& object_name)
      : bfd_(NULL),
        symtab_(NULL),
        dynamic_symtab_(NULL),
        text_(NULL),
        debug_link_(NULL) {
    init(object_name);
  }

  ~BfdSymbols() {
    if (symtab_ != NULL) {
      free(symtab_);
    }
    if (dynamic_symtab_ != NULL) {
      free(dynamic_symtab_);
    }
    if (bfd_ != NULL) {
      bfd_close(bfd_);
    }
  }

  // Resolve given address into a symbol description.
  bool resolve(void *address,
               void *base_address,
               Symbol *symbol) {
    if (bfd_ == NULL) {
      return false;
    }
    SymbolInfo info = symbol_find_info(address, base_address);
    if (info.found) {
      symbol->address = (size_t)address;
      if (info.file_name != NULL) {
        symbol->file_name = info.file_name;
        if (info.line_number != 0) {
          symbol->line_number = info.line_number;
        } else {
          symbol->line_number = Symbol::LINE_NONE;
        }
      }
      if (info.function_name != NULL) {
        symbol->function_name = demangle(info.function_name);
      }
      return true;
    }
    return false;
  }

 protected:
  // Used by symbol_find_info().
  struct SymbolInfo {
    bool found;
    const char *file_name;
    const char *function_name;
    unsigned int line_number;

    SymbolInfo()
    : found(false),
      file_name(NULL),
      function_name(NULL),
      line_number(-1) {}
  };

  struct SymbolInfoData {
    BfdSymbols *self;
    bfd_vma address;
    bfd_vma base_address;
    SymbolInfo info;
  };

  // Initialize symbols information from a given object.
  bool init(const string& object_name) {
    // Make sure library itself it initialized.
    init_bfd_lib();

    // Cretae a BFD descriptor.
    bfd_ = bfd_openr(object_name.c_str(), 0);
    if (bfd_ == NULL) {
        return false;
    }
    // Sanity checks on the binary, to make sure all information
    // is available and symbolizer can be used.
    if (!bfd_check_format(bfd_, bfd_object)) {
      return false;
    }
    if ((bfd_get_file_flags(bfd_) & HAS_SYMS) == 0) {
      return false;
    }
    // Gather information about symbol tables.
    long symtab_size = bfd_get_symtab_upper_bound(bfd_);
    long dynamic_symtab_size = bfd_get_dynamic_symtab_upper_bound(bfd_);
    if (symtab_size <= 0 && dynamic_symtab_size <= 0) {
      return false;
    }
    // Gather symbol tables themselves.
    size_t symtab_count = 0;
    if (symtab_size > 0) {
      symtab_ = reinterpret_cast<asymbol **>(malloc(symtab_size));
      symtab_count = bfd_canonicalize_symtab(bfd_, symtab_);
    }
    if (dynamic_symtab_size > 0) {
      dynamic_symtab_ = reinterpret_cast<asymbol **>(malloc(symtab_size));
      symtab_count = bfd_canonicalize_symtab(bfd_, dynamic_symtab_);
    }
    (void) symtab_count;  // Currently ignored.
    // Gather .text section.
    text_ = bfd_get_section_by_name(bfd_, ".text");
    debug_link_ = bfd_get_section_by_name(bfd_, GNU_DEBUGLINK);
    return true;
  }

  // Initialize BFD library itself.
  void init_bfd_lib() {
    static bool bfd_lib_initialized = false;
    if (!bfd_lib_initialized) {
      bfd_init();
      bfd_lib_initialized = true;
    }
  }

  // Get object name of ourselves.
  string self_object_name_get() {
    char exe_name[PATH_MAX];
    int res = readlink("/proc/self/exe", exe_name, sizeof(exe_name));
    if (res == -1) {
        return "";
    }
    exe_name[res] = 0;
    return exe_name;
  }

  static void symbol_find_info_cb(bfd * /*bfd*/,
                                  asection *section,
                                  void *data_v) {
    SymbolInfoData *data = reinterpret_cast<SymbolInfoData *>(data_v);
    data->self->symbol_find_info_in_section(section, data);
  }

  void symbol_find_info_in_section(asection *section,
                                   SymbolInfoData *data) {
    SymbolInfo& info = data->info;
    // Information was already found.
    if (info.found) {
      return;
    }
    // Debug section is never loaded automatically.
    if ((bfd_get_section_flags(bfd_, section) & SEC_ALLOC) == 0) {
      return;
    }
    bfd_vma address = data->address;
    bfd_vma section_address = bfd_get_section_vma(bfd_, section);
    bfd_size_type section_size = bfd_get_section_size(section);
    if (address < section_address ||
        address >= section_address + section_size) {
      // Relocated object, remap address and try again.
      address -= data->base_address;
      if (address < section_address ||
          address >= section_address + section_size) {
        return;
      }
    }

    if (debug_link_ != NULL) {
      // Workaround memory leak in libbfd.
      return;
    }

    if (symtab_ != NULL) {
      info.found = bfd_find_nearest_line(bfd_,
                                         section,
                                         symtab_,
                                         address - section_address,
                                         &info.file_name,
                                         &info.function_name,
                                         &info.line_number);
    }
    if (!info.found && dynamic_symtab_) {
      info.found = bfd_find_nearest_line(bfd_,
                                         section,
                                         dynamic_symtab_,
                                         address - section_address,
                                         &info.file_name,
                                         &info.function_name,
                                         &info.line_number);
    }
  }

  SymbolInfo symbol_find_info(void *address, void *base_address) {
    SymbolInfoData data;
    data.self = this;
    data.address = (bfd_vma)address;
    data.base_address = (bfd_vma)base_address;
    bfd_map_over_sections(bfd_,
                          &symbol_find_info_cb,
                          reinterpret_cast<void *>(&data));
    return data.info;
  }

  bfd *bfd_;
  asymbol **symtab_;
  asymbol **dynamic_symtab_;
  asection *text_, *debug_link_;
};

// Sybolize implementation using BFD library.
class SymbolizeBfd : public Symbolize {
 public:
  SymbolizeBfd() : Symbolize() {
  }

  explicit SymbolizeBfd(StackTrace *stacktrace)
      : Symbolize(stacktrace) {
    if (stacktrace_ != NULL) {
      resolve(*stacktrace_);
    }
  }

  ~SymbolizeBfd() {
    for (BfdObjectMap::iterator it = bfd_object_map_.begin();
        it != bfd_object_map_.end();
        ++it) {
      delete it->second;
    }
  }

  void resolve(const StackTrace& stacktrace) {
    symbols_.resize(stacktrace.size());
    for (size_t i = 0; i < stacktrace.size(); ++i) {
      unsigned char *address =
              reinterpret_cast<unsigned char*>(stacktrace[i].address);
      resolve(reinterpret_cast<void*>(address - 1), &symbols_[i]);
    }
  }

 private:
  typedef map<string, BfdSymbols*> BfdObjectMap;

  BfdSymbols& load_object(const string& object_name) {
    using std::pair;
    BfdObjectMap::iterator it = bfd_object_map_.find(object_name);
    if (it != bfd_object_map_.end()) {
      return *it->second;
    }
    BfdSymbols *bfd_symbols = new BfdSymbols(object_name);
    bfd_object_map_.insert(pair<string, BfdSymbols*>(object_name, bfd_symbols));
    return *bfd_symbols;
  }

  // Perform all the magic to resolve information about particular address.
  bool resolve(void *address, Symbol *symbol) {
    Dl_info symbol_info;
    if (dladdr(address, &symbol_info) == 0) {
      return false;
    }
    if (symbol_info.dli_fname == NULL) {
      return false;
    }

    BfdSymbols& bfd_symbols = load_object(symbol_info.dli_fname);
    if (symbol_info.dli_sname != NULL) {
      symbol->object_name = symbol_info.dli_sname;
    }
    if (!bfd_symbols.resolve(address,
                             symbol_info.dli_fbase,
                             symbol)) {
      // Fallback mode if bfd resolve fails.
      symbol->address = (size_t)address;
      if (symbol_info.dli_sname != NULL) {
        symbol->function_name = demangle(symbol_info.dli_sname);
      }
    }
    return true;
  }

  BfdObjectMap bfd_object_map_;
};

}  // namespace

Symbolize *symbolize_create_bfd(StackTrace *stacktrace) {
  return new SymbolizeBfd(stacktrace);
}

}  // namespace internal
}  // namespace bt

#endif  // BACKTRACE_HAS_BFD
