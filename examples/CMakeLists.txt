# The essential stuff:
include_directories("..")
add_executable(Romania Romania.cpp)
# add_executable(TSP TSP.cpp)
add_executable(random random.cpp)

# Should I want to use Intel's scalable_allocator from TBB:
# Link with TBB if it is installed and ignore it otherwise.
include(FindPkgConfig)
pkg_check_modules(TBBMALLOC tbbmalloc)
include_directories(${TBBMALLOC_INCLUDE_DIRS})
link_directories(${TBBMALLOC_LIBRARY_DIRS})
# target_link_libraries(TSP ${TBBMALLOC_LIBRARIES})
