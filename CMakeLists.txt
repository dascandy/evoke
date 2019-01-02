cmake_minimum_required(VERSION 3.0.0)
project(evoke)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Disable autolinking on Windows because it conflicts with FIND_BOOST.
ADD_DEFINITIONS("-DBOOST_ALL_NO_LIB")
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Prevented in-tree build. Please create a build directory outside of the source directory and call cmake from there.")
endif()

set(INCLUDES  ${CMAKE_CURRENT_SOURCE_DIR}/fw/include 
              ${CMAKE_CURRENT_SOURCE_DIR}/project/include
              ${CMAKE_CURRENT_SOURCE_DIR}/toolsets/include 
              ${CMAKE_CURRENT_SOURCE_DIR}/view/include 
              ${CMAKE_CURRENT_SOURCE_DIR}/evoke/include 
              ${CMAKE_CURRENT_SOURCE_DIR}/reporter/include
              ${CMAKE_CURRENT_SOURCE_DIR}/utils/include)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/toolsets/src/known.cpp 
            ${CMAKE_CURRENT_SOURCE_DIR}/toolsets/src/Toolset.cpp 
            ${CMAKE_CURRENT_SOURCE_DIR}/toolsets/src/gcc.cpp 
            ${CMAKE_CURRENT_SOURCE_DIR}/toolsets/src/clang.cpp 
            ${CMAKE_CURRENT_SOURCE_DIR}/toolsets/src/msvc.cpp 
            ${CMAKE_CURRENT_SOURCE_DIR}/toolsets/src/android.cpp 
            ${CMAKE_CURRENT_SOURCE_DIR}/fw/src/Configuration.cpp 
            ${CMAKE_CURRENT_SOURCE_DIR}/fw/src/dotted.cpp 
            ${CMAKE_CURRENT_SOURCE_DIR}/project/src/Component.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/project/src/CMakeListsTxt.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/project/src/JsonCompileDb.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/project/src/PendingCommand.cpp 
            ${CMAKE_CURRENT_SOURCE_DIR}/project/src/PredefComponents.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/project/src/Project.cpp 
            ${CMAKE_CURRENT_SOURCE_DIR}/project/src/ReadCode.cpp 
            ${CMAKE_CURRENT_SOURCE_DIR}/project/src/tarjan.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/reporter/src/ConsoleReporter.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/reporter/src/SimpleReporter.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/reporter/src/Reporter.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/evoke/src/main.cpp 
            ${CMAKE_CURRENT_SOURCE_DIR}/evoke/src/Executor.cpp)

find_package(Boost REQUIRED COMPONENTS system filesystem)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES} ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
