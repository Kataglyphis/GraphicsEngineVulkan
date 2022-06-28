# https://cmake.org/cmake/help/latest/module/ProcessorCount.html

include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
  set(CTEST_BUILD_FLAGS -j${N})
  set(ctest_test_args ${ctest_test_args} PARALLEL_LEVEL ${N})
  set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
endif()