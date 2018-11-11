#include "Toolset.h"

//https://blogs.msdn.microsoft.com/vcblog/2015/12/03/c-modules-in-vs-2015-update-1/

// Enable modules support for MSVC
//"/experimental:module /module:stdIfcDir \"$(VC_IFCPath)\" /module:search obj/modules/"


// Tell MSVC to act like a sane compiler (use the latest C++ version in standards conforming mode, use C++ exceptions as specified, and link to the C/C++ runtime)
//"/std:c++latest /permissive- /EHsc /MD"

void WindowsToolset::CreateCommandsFor(Project& project) {
  std::abort();
}



