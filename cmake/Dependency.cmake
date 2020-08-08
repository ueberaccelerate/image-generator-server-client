include ( cmake/CPM.cmake )

find_package(Boost 1.73.0 COMPONENTS program_options REQUIRED )

CPMAddPackage(
  NAME Catch2
  GITHUB_REPOSITORY catchorg/Catch2
  VERSION 2.5.0
)

CPMAddPackage(
  NAME benchmark
  GITHUB_REPOSITORY google/benchmark
  VERSION 1.4.1
  OPTIONS
    "BENCHMARK_ENABLE_TESTING Off"
	"BENCHMARK_ENABLE_EXCEPTIONS Off"
	"BENCHMARK_ENABLE_GTEST_TESTS Off"
	"BENCHMARK_ENABLE_INSTALL Off"
)

CPMAddPackage(
  NAME libprop
  GITHUB_REPOSITORY ueberaccelerate/libprop
  GIT_TAG 0e05cfb 
  OPTIONS
    "BUILD_TESTING Off"
#    "EXAMPLES_ENABLED Off"
#    "TESTS_ENABLED  Off"
#    "QT_EDITOR_ENABLED Off"
)


if (benchmark_ADDED)
  # compile with C++17
  set_target_properties(benchmark PROPERTIES CXX_STANDARD 17)
endif()
