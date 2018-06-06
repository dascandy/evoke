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
  "stdio.h", "conio.h", "assert.h", "ctype.h", "cocale.h", "math.h", "setjmp.h", "signal.h", "stdarg.h", "stdlib.h", "string.h", "time.h", "complex.h", "stdalign.h", "errno.h", "locale.h", "stdatomic.h",
  "stdnoreturn.h", "uchar.h", "fenv.h", "wchar.h", "tgmath.h", "stdarg.h", "stdbool.h",

  // Linux system headers
  "sys/types.h",
  "sys/wait.h",
  "sys/mman.h",

  // Unix system headers
  "fcntl.h",
  "unistd.h",

  "stddef.h"
  "stdint.h",
  "limits.h",
  "iso646.h",
  "wctype.h",
  "inttypes.h",
  "malloc.h",

/*
afxcoll.h
afx.h
atlbase.h
atldef.h
atlsimpstr.h
atomic.h
basetyps.h
bcrypt.h
bits/atomicity.h
bits/char_traits.h
bits/fenv.h
bits/move.h
bits/stl_algobase.h
bits/stl_function.h
bits/stl_move.h
bits/stl_pair.h
builtins.h
cxxabi.h
dbghelp.h
dirent.h
ext/atomicity.h
features.h
float.h
ia64intrin.h
imagehlp.h
../include/fenv.h
intrin.h
io.h
ioLib.h
kfuncs.h
libs/regex/test/config_info/regex_config_info.cpp
limits.h
machine/endian.h
machine/sys/inline.h
_mingw.h
msl_utility
ntverp.h
ostream.h
process.h
pthread.h
sched.h
shellapi.h
sys/atomic_op.h
sys/cygwin.h
sys/endian.h
sysLib.h
sys/mount.h
sys/param.h
sys/stat.h
sys/statvfs.h
sys/time.h
sys/utime.h
sys/vfs.h
TargetConditionals.h
tchar.h
thread.h
tickLib.h
type_traits.h
unicode/coll.h
unicode/uchar.h
unicode/utypes.h
utime.h
winapifamily.h
wincrypt.h
winerror.h
winnt.h
xutility
*/
};

bool IsKnownHeader(const std::string& str) {
  return known.find(str) != known.end();
}


