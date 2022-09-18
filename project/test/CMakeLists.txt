cmake_minimum_required(VERSION 3.9)
project(evoke_test_project)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree build. Please create a build directory outside of the source directory and call cmake from there.")
endif()

set(INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../evoke/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../fw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../project/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../reporter/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../toolsets/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../view/include
)


set(LINKED_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Component.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/PendingCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../fw/src/dotted.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/File.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../fw/src/sha512.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ReadCode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Utilities.cpp
)

set(SOURCES
    main.cpp ComponentTest.cpp UtilitiesTest.cpp ReadCodeFromTest.cpp
)

if(MSVC)
    source_group("Linked Source Files" FILES ${LINKED_SOURCES})
endif(MSVC)

add_executable(${PROJECT_NAME} ${SOURCES} ${LINKED_SOURCES})
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /permissive-)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -D_WIN32_WINNT=0x0601 -D_CRT_SECURE_NO_WARNINGS)
endif()

if (MINGW)
    set(Boost_ARCHITECTURE -x64)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system unit_test_framework)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::disable_autolinking OpenSSL::SSL OpenSSL::Crypto)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES} ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads ${Boost_LIBRARIES})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${PROJECT_NAME} COMMENT "Running unit tests")
