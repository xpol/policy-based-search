# The essential stuff:
# Enforce latest language standards.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

include_directories("..", "utils")
add_executable(Romania Romania.cpp)
add_executable(TSP TSP.cpp)

# Should I want to use Intel's scalable_allocator from TBB:
# This is meant to link with TBB, if it is installed and ignore it otherwise.
include(FindPkgConfig)
pkg_check_modules(TBBMALLOC tbbmalloc)
include_directories(${TBBMALLOC_INCLUDE_DIRS})
link_directories(${TBBMALLOC_LIBRARY_DIRS})
target_link_libraries(TSP ${TBBMALLOC_LIBRARIES})
