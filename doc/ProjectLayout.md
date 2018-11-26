# Project layout

Evoke relies on your projects having a source-derivable structure and componentization. This document describes the requirements on that layout, and the known restrictions currently in place.

# Components

Your source tree should consist of a number of components. Each component is identified by having a `src` or `include` folder - possibly both - which contains the to-be-compiled code. The component name Evoke uses to talk about this is derived from the path, by replacing directory separators with dots. The target file(s) are derived from the component name by applying the platform common transformation - libproject.a on Linux, project.lib on Windows. 

A component can be compiled as one of three types - a static library, a dynamic library, or an executable. Evoke auto-detects which type it should be by looking at the dependencies coming into the target:

- If there is no dependency coming into the target, it will be an executable.
- If there is any dependency coming into the target (including from its own unit tests), it will be a static library.

These will be overrideable, for example for libraries that are currently just not used.

Tests are always found next to the component they are testing. The test is in a folder called `test` next to the component's `src` or `include` folder. The files in this folder are compiled as per the rules for an executable, and the executable is run from the project root (*this behavior may change in the future*). The build is considered failed if the command creates a failed error code (* currently not exported*).

# Internal dependencies

Project internal dependencies are found and resolved automatically, with the appropriate include paths being set for each component's compile commands.

In case an `#include` or `import` statement can resolve to two or more files depending on include paths; the include itself is regarded as ambiguous, and its potential targets are not taken into account for include path calculation and dependencies. This will lead to build failures if you have multiple of these.

# External dependencies

Currently external dependencies are not handled well at all yet. For about 6 targets there is a provisional setup in `Project.cpp` that provides sufficient information to use them, given that they are installed system-wide. Any package manager that fetches packages into a project subfolder wholly including sources will also work, as it sees this as just another (internal) component and act accordingly.

We are actively looking at finding tools that, given an include that does not resolve internally, can search for the appropriate package(s) and indirect dependencies of those packages to fetch. There is a hook for it in `evoke/src/main.cpp` that points to `accio`, which is a currently still private project that does this (but very unreliably right now).

# Compile and link flags

Evoke does not right now allow customization of the compiler and linker flags. It uses defaults that match "current" compilers. This will be picked up together with externalizing toolsets as they both use the same mechanism to customize. The default set used right now is to turn on a reasonable set of warnings (`-Wall -Wextra`) and to set the standard to C++17 (`-std=c++17`).

