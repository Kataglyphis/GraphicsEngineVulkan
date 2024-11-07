include(GoogleTest)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "This is a Linux system.")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
      
      if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        fuzztest_setup_fuzzing_flags()
      endif()
    
      add_executable(
        first_fuzz_test
        dummy.cpp
      )
      
      link_fuzztest(first_fuzz_test)
      gtest_discover_tests(first_fuzz_test)
    endif()
endif()