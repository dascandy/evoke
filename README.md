# Evoke

Evoke is the simple solution to the complicated problem of build software for C++ and related languages.

# Building/installing

Right now it is not available as a package for common operating systems yet. To install it, you will need to compile it from source.

It requires Boost 1.60 (or thereabouts) or higher; it uses only boost.filesystem. To compile, download the full source tree and type `make` in the place you downloaded it to. Then, run `bin/evoke_make` to build evoke using itself. When this succeeds, you will have a `bin/evoke` that does the same thing, but is built with Evoke. Copy this to `~/bin/evoke` for a user-local installation or to `/usr/local/bin/evoke` for a system-wide installation.

# Using Evoke

To use evoke, create a folder that is named after your project target, and create a `src` or `include` folder inside that. Evoke recognizes a `src` or `include` folder as the root of a component, and will name it after the directory tree navigated to get to it. For example, if you have a folder called `hello/src` it will create a component called `hello`, and if your folder is called `thirdparty/catch/include` it will create a component called `thirdparty.catch`.

Evoke reads all the source code found inside components this way and analyzes their dependencies through use of `#include` and `import` statements. It derives the full dependency tree of the source tree and uses it to determine which components will become a library and which will become an executable. Its rule is that any component *with* a link coming from something else must be a library (because something else is including its files), and any component *without* a link coming from something else must be an executable. To compile, simply type `evoke` at the root of your project and it will compile the full set of source files with appropriate flags for the current version of c++ into libraries and executables.

Still to be implemented is minor customization around this:
- It needs to allow overriding for when you have an unused library (that it will now compile as an executable) and when you want to use shared libraries.
- It needs to allow customization of compile flags for the project-wide compilation, so that you can disable some warnings.
- It needs to allow customization of the toolchain, so that you can use it for cross-compiling to other targets, or tune it for some CPU type.

# Developing Evoke

Everybody is free to help with Evoke development. The simpler things that need to be done are to create issues for things you would like it to do, or for asking help when it does not do what you want it to. You can join the discord at https://includecpp.org . If you want to do more, there are a few open issues already that require a bit more knowledge and time investment, like porting it to run on Windows or OSX.

### Clang Format 

Formatting of all source files is done using ClangFormat. Rules for it are specified in .clang-format file in the root of the repository.

**Path:** /.clang-format

**Requires:** [LLVM](http://llvm.org/) version 6.0

# License

Evoke is partially derived from https://github.com/tomtom-international/cpp-dependencies (Apache2 licensed) and partially from https://github.com/dascandy/bob (my copyright, also Apache2 unless I'm mistaking). Because of that it is also Apache2 licensed. If this does not work for you for some reason, reach out and I'll see what we can do/change.

