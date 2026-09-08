# Mortgage Testing Lab
# Osvaldo Janeri Filho <janeri@gmail.com>
#
# Builds the mortgage library and four test executables, one per testing
# technique. The whole file exists to support one trick: the same test binaries
# can be built against either implementation, chosen by a flag, so the suite
# never has to be edited to make it fail.

cmake_minimum_required(VERSION 3.14)  # 3.14 is where FetchContent_MakeAvailable arrived
project(mortgage_testing_lab LANGUAGES CXX)

# C++17, and REQUIRED means fail loudly at configure time rather than silently
# falling back to an older standard. EXTENSIONS OFF asks for -std=c++17 rather
# than -std=gnu++17, so the code stays portable instead of quietly depending on
# a GCC extension.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Which implementation goes under test
# ---------------------------------------------------------------------------
#
#   cmake -S . -B build                     -> src/mortgage.cpp, suite is GREEN
#   cmake -S . -B build-buggy -DUSE_BUGGY=ON -> src/mortgage_buggy.cpp, RED
#
# Both .cpp files define the same `int mortgage(Gender, int, int)` declared in
# include/mortgage.hpp, so only one of them can be in the library at a time —
# linking both would be a duplicate symbol error. That constraint is what makes
# the swap clean: there is no #ifdef anywhere in the source, and the tests
# cannot tell which version they are running against except by how it behaves.
option(USE_BUGGY "Link the intentionally defective implementation" OFF)

if(USE_BUGGY)
  set(MORTGAGE_SOURCE src/mortgage_buggy.cpp)
  message(STATUS "mortgage: linking the BUGGY implementation (expect failures)")
else()
  set(MORTGAGE_SOURCE src/mortgage.cpp)
  message(STATUS "mortgage: linking the CORRECT implementation")
endif()

# ---------------------------------------------------------------------------
# Warning flags
# ---------------------------------------------------------------------------
#
# An INTERFACE library carries no code of its own; it exists purely to hand its
# settings to whatever links it. Doing it this way means the library AND the
# four test binaries all get the same flags from one place.
#
# This used to be `target_compile_options(mortgage PRIVATE -Wall -Wextra)`,
# which put the flags on the library only. PRIVATE means "for building me, not
# for anyone who links me", so the tests were compiling with no warnings
# enabled at all. Caught during review.
add_library(mortgage_warnings INTERFACE)
if(MSVC)
  target_compile_options(mortgage_warnings INTERFACE /W4)
else()
  target_compile_options(mortgage_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# The library under test
# ---------------------------------------------------------------------------
add_library(mortgage ${MORTGAGE_SOURCE})

# PUBLIC on the include directory: `mortgage` needs it to compile, and so does
# anything that links `mortgage`. That is what lets the test files write
# #include "mortgage.hpp" without any path of their own.
target_include_directories(mortgage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# PRIVATE on the warnings: they apply while building this library, but nothing
# that links it inherits them. Each test target asks for them separately below.
target_link_libraries(mortgage PRIVATE mortgage_warnings)

# ---------------------------------------------------------------------------
# GoogleTest
# ---------------------------------------------------------------------------
#
# FetchContent downloads and builds GoogleTest as part of this project, so
# nothing has to be installed on the machine first. The version is pinned to a
# tag rather than a branch: an unpinned dependency means a build that passes
# today and fails next month for reasons that have nothing to do with the code.
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
  GIT_SHALLOW    TRUE  # only that one tag, not the whole history
)
# Windows-only setting, harmless elsewhere: makes GoogleTest use the same
# runtime library as the rest of the build instead of its own default.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ---------------------------------------------------------------------------
# The test executables
# ---------------------------------------------------------------------------
enable_testing()        # turns on CTest for this project
include(GoogleTest)     # brings in gtest_discover_tests

# One binary per technique rather than one big binary, so a failure line from
# ctest already tells you which technique caught the bug before you read the
# test name.
set(MORTGAGE_TEST_SUITES
  nominal_test
  boundary_test
  robustness_test
  equivalence_test
)

foreach(suite ${MORTGAGE_TEST_SUITES})
  add_executable(${suite} tests/${suite}.cpp)

  # gtest_main provides main(), so the test files contain only TEST blocks.
  target_link_libraries(${suite} PRIVATE mortgage mortgage_warnings GTest::gtest_main)

  # Runs the binary at build time with --gtest_list_tests and registers each
  # TEST as its own CTest entry. That is why `ctest` reports 44 tests rather
  # than 4 — it can name the individual case that failed.
  gtest_discover_tests(${suite})
endforeach()
