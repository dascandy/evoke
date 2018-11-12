# CLion usage

There are two ways to use Evoke from within CLion. Sadly, they are basically complementary and neither of them works quite as we'd want.

# CMake-style

CLion is built around importing and using CMake projects. There is a CMakeLists.txt in the root that tells CLion to invoke Evoke from CMake, which then compiles the actual project. You can use this in any project, but it has the downside that CLion doesn't think any targets are defined and won't do any auto-completion. It will, however, let you build the project.

# Compilation database style

CLion also supports importing a compilation database. This has the upside that CLion knows exactly which command to invoke after a given file was edited, so it can provide immediate feedback on changes, and it knows the relevant include paths so it can provide good autocompletion. The downside is that in this mode, CLion does not allow adding a build command (as far as I know), so it will refuse to build the project.

# If you know more than we do

Please let us know. We want to have a better integration with CLion but currently we have no idea how to.
