# Global flow

The tool starts in [evoke/src/main.cpp](../evoke/src/main.cpp). It reads command-line arguments and then constructs a Project object. This object's constructor collects the whole project status & information immediately. Directly after this point it runs the actions that the user instructed it to do. This part of the program may be changed; right now it offers a project dump in verbose mode, a compilation database dump and running the build itself with the built-in executor.

Directly after creating this, the main loop uses a toolchain (from [toolsets/src/Toolset.cpp](../toolsets/src/Toolset.cpp)) to determine what commands need to be run and what their rebuild-if-changed dependencies are. These are then fed to the executor to run in any order it likes.

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

# Design principles

## Componentization required by Evoke

Evoke requires you to restructure your code base, so that the code base structure explains how the code is laid out. After this, you can still use any descriptive build system to build the code, but you can also use Evoke to build it without adding configuration. It additionally means that anybody who knows the project layout method can read your code base to see what does what, and any tool using it will instantly know what is used where and how things are to be used, created and built. The idea is not to make a better configuration file, but to make it unnecessary.

The idea is that a project buildable with Evoke is subdivided into components, which themselves do not have an internal subdivision. So a component is always either fully included, or not included, and at the same time if you include a header from a component you should logically need to use the whole component. Those components are called coherent.

As far as Evoke is concerned, your folders are arbitrarily nested and contain components. You can have inner components, you can have any levels deep of components, you can have cyclic dependencies, you can have simple dependencies. It really does not care how you structure your components or dependencies themselves - that's up to you.  I would personally argue against inner components and cyclic dependencies, but there's no technical reason in Evoke they wouldn't work, so they just work. In the case of cyclic dependencies, the code base explicitly creates linker lines that always work when you have them (rather than the repeat-N-times thing that CMake does).

Supporting large projects is the same as small projects; you're just more likely to accidentally have created problems in a large project. Dependency detection works the same, ambiguous headers are still ambiguous, projects that somebody includes a header of still are libraries.

## Component detection

Evoke requires

- Each component has a `src` or `include` folder at its root
- The unit test for a component is in a folder `test` next to its `src` and/or `include` folders.
- All files found within a component, but not more closely in another component, are part of it

The names of these folders are not modifiable - adding such a capability is not all that hard, but would remove much of the benefit you get from the recognizability of the code base structure.


