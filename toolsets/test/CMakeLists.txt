cmake_minimum_required(VERSION 3.9)
project(evoke_test_toolsets)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(
    FATAL_ERROR
    "Prevented in-tree build. Please create a build directory outside of the source directory and call cmake from there."
  )
endif()


set(INCLUDES
    ../../evoke/include
    ../../fw/include
    ../../project/include
    ../../reporter/include
    ../../toolsets/include
    ../../utils/include
    ../../view/include
)

set(LINKED_SOURCES
    ../../project/src/PendingCommand.cpp
    ../../project/src/PredefComponents.cpp
    ../../project/src/ReadCode.cpp
    ../../project/src/Project.cpp
    ../../project/src/Component.cpp
    ../../project/src/tarjan.cpp
    ../../project/src/File.cpp
    ../../project/src/Utilities.cpp
    ../../project/src/known.cpp
    ../../fw/src/dotted.cpp
    ../../fw/src/FileParser.cpp
    ../src/msvc.cpp
    ../src/Toolset.cpp
    ../src/generic.cpp
    ../src/clang.cpp
    ../src/gcc.cpp
 )

set(SOURCES
    main.cpp
    msvc_test.cpp
    clang_test.cpp
    gcc_test.cpp
)

if(MSVC)
  source_group("Linked Source Files" FILES ${LINKED_SOURCES})
endif(MSVC)

add_executable(${PROJECT_NAME} ${SOURCES} ${LINKED_SOURCES})

if (MINGW)
  set(Boost_ARCHITECTURE -x64)
  target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(Boost REQUIRED COMPONENTS filesystem system unit_test_framework)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES} ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads ${Boost_LIBRARIES} Boost::disable_autolinking)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${PROJECT_NAME} COMMENT "Running unit tests")
