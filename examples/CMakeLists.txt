# The essential stuff:
include_directories("..", "utils")
add_executable(Romania Romania.cpp)
add_executable(TSP TSP.cpp)
# add_executable(random random.cpp)
# add_executable(test test.cpp)
# Should I want to use Intel's scalable_allocator from TBB:
include(FindPkgConfig)
pkg_check_modules(TBBMALLOC tbbmalloc)
include_directories(${TBBMALLOC_INCLUDE_DIRS})
link_directories(${TBBMALLOC_LIBRARY_DIRS})
target_link_libraries(TSP ${TBBMALLOC_LIBRARIES})
# set(LD_PRELOAD "libtbbmalloc_proxy.so.2")
