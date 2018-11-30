# Command-line usage

Evoke's typical use is from a command-line. To run it, type the command "evoke" and your libraries and executables will be built, and your tests will be run.

# Arguments

Evoke supports a few arguments. This list will grow over the next few months.

`-v`: Output verbose compilation information about the projects, including the full set of commands to be run and all inter-component dependencies.
`-cp`: Create a compilation database in the root with the name `compile_database.json`. This can be used for CLion or other tools that can use a compile database.
`--root`: Indicate to Evoke that the root folder of the project is not the current folder, but the indicated folder instead.
`--cm`: Output a basic set of CMakeList.txt files so that the project can be built with CMake.

