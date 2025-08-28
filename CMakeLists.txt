cmake_minimum_required(VERSION 3.20)
project(MPMCQueue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Library (your code) ----
# List sources explicitly (best for clarity)
# set(YOURLIB_SOURCES
#   src/MPMCQueue_main.cpp
# )
# set(YOURLIB_PUBLIC_HEADERS
#   include/MPMCQueue.h
#   include/Person.h
# )

# add_library(yourlib ${YOURLIB_SOURCES} ${YOURLIB_PUBLIC_HEADERS})
# add_library(yourlib::yourlib ALIAS yourlib)

# target_include_directories(yourlib
#   PUBLIC
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#     $<INSTALL_INTERFACE:include>
# )

# ---- Header-only library (all headers under include/) ----
add_library(mpmcqueue INTERFACE)
target_include_directories(mpmcqueue INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# ---- Google Benchmark ----
include(FetchContent)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.4
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark)

# ---- Benchmark executable ----
#file(GLOB BENCH_SOURCES CONFIGURE_DEPENDS bench/*.cpp)
set(BENCH_SOURCES
  #src/MPMCQueue_main.cpp)
  src/MPMCQueue_bench.cpp)


find_package(Threads REQUIRED)

add_executable(mpmc_bench ${BENCH_SOURCES})
target_link_libraries(mpmc_bench PRIVATE
  mpmcqueue
  benchmark::benchmark
  Threads::Threads
)

# Windows-specific fixes
target_compile_definitions(mpmc_bench PRIVATE BENCHMARK_STATIC_DEFINE)
if (WIN32)
  target_link_libraries(mpmc_bench PRIVATE Shlwapi)
endif()


set(MAIN_SOURCES
  src/MPMCQueue_main.cpp)
add_executable(mpmcq_main ${MAIN_SOURCES})
target_link_libraries(mpmcq_main PRIVATE
	mpmcqueue
	Threads::Threads)
