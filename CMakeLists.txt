cmake_minimum_required(VERSION 2.8.4)
project(mytarget)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

add_custom_target(mytarget 
  COMMAND evoke --root ${PROJECT_SOURCE_DIR}
)

