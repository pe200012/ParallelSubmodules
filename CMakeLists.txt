cmake_minimum_required (VERSION 3.14)
project(ParallelSubmodules)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 20)

find_package (Boost REQUIRED COMPONENTS filesystem)
set(Boost_USE_MULTITHREADED ON)

add_subdirectory (third_party/asyncplusplus)

include_directories(${Boost_INCLUDE_DIRS}
  ${ASYNCXX_INCLUDES}
  third_party
  )
add_executable(psubm
  main.cpp
  )
add_executable(patest
  patest.cpp
  )
target_link_libraries(psubm ${Boost_LIBRARIES} Async++)
target_link_libraries(patest Async++)
