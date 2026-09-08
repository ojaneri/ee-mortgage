cmake_minimum_required(VERSION 3.14)
project(mortgage_testing_lab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Which implementation is under test?
#   -DUSE_BUGGY=OFF (default) -> src/mortgage.cpp        : suite must be GREEN
#   -DUSE_BUGGY=ON            -> src/mortgage_buggy.cpp  : suite must be RED
# ---------------------------------------------------------------------------
option(USE_BUGGY "Link the intentionally defective implementation" OFF)

if(USE_BUGGY)
  set(MORTGAGE_SOURCE src/mortgage_buggy.cpp)
  message(STATUS "mortgage: linking the BUGGY implementation (expect failures)")
else()
  set(MORTGAGE_SOURCE src/mortgage.cpp)
  message(STATUS "mortgage: linking the CORRECT implementation")
endif()

add_library(mortgage ${MORTGAGE_SOURCE})
target_include_directories(mortgage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(MSVC)
  target_compile_options(mortgage PRIVATE /W4)
else()
  target_compile_options(mortgage PRIVATE -Wall -Wextra)
endif()

# ---------------------------------------------------------------------------
# GoogleTest v1.15.2 via FetchContent
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

# One executable per functional-testing technique, so a RED run tells you
# immediately which technique caught the defect.
set(MORTGAGE_TEST_SUITES
  nominal_test
  boundary_test
  robustness_test
  equivalence_test
)

foreach(suite ${MORTGAGE_TEST_SUITES})
  add_executable(${suite} tests/${suite}.cpp)
  target_link_libraries(${suite} PRIVATE mortgage GTest::gtest_main)
  gtest_discover_tests(${suite})
endforeach()
