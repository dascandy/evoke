#include "known.h"
#include <unordered_set>

static std::unordered_set<std::string> known = {
  // C++ standard headers
  "algorithm", "any", "array", "atomic", "bitset", "charconv", "chrono", "codecvt", "compare", "complex", "concepts", "condition_variable", "deque", "exception", "execution", "filesystem", "forward_list",
  "fstream", "functional", "future", "initializer_list", "iomanip", "ios", "iosfwd", "iostream", "istream", "iterator", "limits", "list", "locale", "map", "memory", "memory_resource", "mutex", "new",
  "numeric", "optional", "ostream", "queue", "random", "range", "ratio", "regex", "scoped_allocator", "set", "shared_mutex", "span", "sstream", "stack", "stdexcept", "streambuf", "string", "string_view",
  "strstream", "syncstream", "system_error", "thread", "tuple", "type_traits", "typeindex", "typeinfo", "unordered_map", "unordered_set", "utility", "valarray", "variant", "vector", "version",

  // c-compat C++ headers
  "cstdio", "cconio", "cassert", "cctype", "ccocale", "cmath", "csetjmp", "csignal", "cstdarg", "cstdlib", "cstring", "ctime", "ccomplex", "cstdalign", "cerrno", "clocale", "cstdatomic", "cstdnoreturn",
  "cuchar", "cfenv", "cwchar", "ctgmath", "cstdarg", "cstdbool", "ciso646", "climits", "cstddef", "cstdint", "cwctype", 

  // C standard library header files
  "assert.h", "cocale.h", "complex.h", "conio.h", "ctype.h", "errno.h", "fenv.h", "inttypes.h", "iso646.h", "limits.h", "locale.h", "malloc.h", "math.h", "setjmp.h", "signal.h", "stdalign.h", "stdarg.h", 
  "stdarg.h", "stdatomic.h", "stdbool.h", "stddef.h", "stdint.h", "stdio.h", "stdlib.h", "stdnoreturn.h", "string.h", "tgmath.h", "time.h", "uchar.h", "wchar.h", "wctype.h",

  // common OS system headers
  "afxcoll.h", "afx.h", "atlbase.h", "atldef.h", "atlsimpstr.h", "atomic.h", "basetyps.h", "bcrypt.h", "builtins.h", "cxxabi.h", "dbghelp.h", "dirent.h", "ext/atomicity.h", "fcntl.h", "features.h", 
  "float.h", "ia64intrin.h", "imagehlp.h", "intrin.h", "io.h", "ioLib.h", "kfuncs.h", "limits.h", "machine/endian.h", "machine/sys/inline.h", "_mingw.h", "msl_utility", "ntverp.h", "ostream.h", 
  "process.h", "pthread.h", "sched.h", "shellapi.h", "strings.h", "sys/atomic_op.h", "sys/cygwin.h", "sys/endian.h", "sysLib.h", "sys/mman.h", "sys/mount.h", "sys/param.h", "sys/stat.h", "sys/statvfs.h", 
  "sys/time.h", "sys/types.h", "sys/utime.h", "sys/vfs.h", "sys/wait.h", "TargetConditionals.h", "tchar.h", "thread.h", "tickLib.h", "type_traits.h", "unistd.h", "utime.h", "winapifamily.h", 
  "wincrypt.h", "winerror.h", "winnt.h", "xutility", "linux/if.h", "netinet/in.h", "linux/if_tun.h", "sys/ioctl.h",


  // Stuff you really shouldn't include
  "bits/atomicity.h", "bits/char_traits.h", "bits/fenv.h", "bits/move.h", "bits/stl_algobase.h", "bits/stl_function.h", "bits/stl_move.h", "bits/stl_pair.h",
};

bool IsKnownHeader(const std::string& str) {
  return known.find(str) != known.end();
}


