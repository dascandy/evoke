# Global flow

The tool starts in evoke/src/main.cpp. It reads command-line arguments and then constructs a Project object. This object's constructor collects the whole project status & information immediately. Directly after this point it runs the actions that the user instructed it to do. This part of the program may be changed; right now it offers a project dump in verbose mode, a compilation database dump and running the build itself with the built-in executor.

Directly after creating this, the main loop uses a toolchain (from `toolsets/src/Toolset.cpp`) to determine what commands need to be run and what their rebuild-if-changed dependencies are. These are then fed to the executor to run in any order it likes.

# Reading the project

Reading the project happens in the Project constructor. 
- It first reads all the files in the source tree that look like a source file and that are not blacklisted. This fills in the `rawImports`, `rawIncludes`, `moduleName` in the File structs. It also recognizes components by the existence of `include` and `src` folders and adds the files to the correct components.
- It creates mappings from symbol-wise import and filename-wise import/include to whichever file it points to. Any entry that can have two or more targets is marked as invalid by overwriting the target with the tag "INVALID".
- It then uses the mappings to find out which files refer to which other files. If the file is being referred to without requiring precompilation, it's added to the `dependencies`, otherwise it's added to the `modImports` list. The latter may (depending on compiler and toolchain) be used directly anyway, but in at least one compiler this causes a build-order dependency that does not exist for the `dependencies` files. At this point, there is no difference between importing by name and importing by filename any more; both need precompilation.
- It propagates file usage information to the component level and extracts what include paths need to be exposed on a component to make the includes/imports work out. This creates a component-level dependency graph too, with (given how files include others) public and private include paths, and public and private component dependencies.

# Expected / planned changes

- The project reading does not fully do modules well; this needs minor fixes to be added still. Depends on how compilers want to receive module information too.
- The whole setup of toolset/ needs to change; this needs to be a file-driven thing where a few default configs are included. Some parts will need to be in code I suspect (as they rely on modImports for example) but most of it should be fine without. This also enables people to add other tools (protobuf compiler, Qt Moc, similar things) to the build setup.
- Unit tests all over the place.
- Online integration tests with Travis and Appveyor.

