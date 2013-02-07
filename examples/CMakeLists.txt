# The essential stuff:
include_directories("..")
add_executable(Romania Romania.cpp)
add_executable(TSP TSP.cpp)
add_executable(random random.cpp)
add_executable(rgg rgg.cpp)

find_package(Boost COMPONENTS graph regex REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(random ${Boost_LIBRARIES})

# Should I want to use Intel's scalable_allocator from TBB:
# Link with TBB if it is installed and ignore it otherwise.
include(FindPkgConfig)
pkg_check_modules(TBBMALLOC tbbmalloc)
include_directories(${TBBMALLOC_INCLUDE_DIRS})
link_directories(${TBBMALLOC_LIBRARY_DIRS})
target_link_libraries(TSP ${TBBMALLOC_LIBRARIES})
