# all testing related stuff
set(WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../)

# update current positions
set(PROJECT_SRC_DIR ${WORKING_DIRECTORY}Src/)
set(PROJECT_INCLUDE_DIR ${WORKING_DIRECTORY}include/)
set(EXTERNAL_LIB_SRC_DIR ${WORKING_DIRECTORY}ExternalLib/)
set(SHADER_SRC_DIR ${WORKING_DIRECTORY}Resources/Shaders/)

include(${WORKING_DIRECTORY}cmake/filters/SetShaderFilters.cmake)
include(${WORKING_DIRECTORY}cmake/filters/SetProjectFilters.cmake)
include(${WORKING_DIRECTORY}cmake/filters/SetExternalLibsFilters.cmake)

include(cmake/SetTestFilters.cmake)

set (COMMIT_TEST_SUITE commitTestSuite)

add_executable( ${COMMIT_TEST_SUITE}
                # all test specific files -- start
                ${RENDERER_COMMIT_TEST_FILTER}
                # all test specific files -- end
                $<TARGET_OBJECTS:PROJECT_FILES_TO_TEST>
                $<TARGET_OBJECTS:IMGUI>
)

target_include_directories(  ${COMMIT_TEST_SUITE} PUBLIC    ${WORKING_DIRECTORY}include/renderer
                                                            ${WORKING_DIRECTORY}include/renderer/pushConstants
                                                            ${WORKING_DIRECTORY}include/renderer/accelerationStructures
                                                            ${WORKING_DIRECTORY}include/vulkan_base
                                                            ${WORKING_DIRECTORY}include/gui
                                                            ${WORKING_DIRECTORY}include/common
                                                            ${WORKING_DIRECTORY}include/scene
                                                            ${WORKING_DIRECTORY}include/window
                                                            ${WORKING_DIRECTORY}include/memory
                                                            ${WORKING_DIRECTORY}include/util
                                                            ${WORKING_DIRECTORY}include/app
                                                            ${WORKING_DIRECTORY}Resources/Shaders/hostDevice
                                                            ${WORKING_DIRECTORY}include/
)

target_include_directories(  ${COMMIT_TEST_SUITE} PRIVATE ${Vulkan_INCLUDE_DIRS})

target_link_libraries( ${COMMIT_TEST_SUITE} PUBLIC  gtest_main
                                                    ${CMAKE_DL_LIBS}   
                                                    Threads::Threads
                                                    Vulkan::Vulkan
                                                    glfw 
                                                    imgui 
                                                    stb
                                                    glm
                                                    tinyobjloader
                                                    vma
                                                    ktx
)

# good source: https://stackoverflow.com/questions/52255589/code-coverage-analysis-using-codecoverage-cmake-with-multiple-targets
option(COVERAGE "Enable Code Coverage" ON)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(COVERAGE)
  include(cmake/CodeCoverage.cmake)
  append_coverage_compiler_flags()
  setup_target_for_coverage_gcovr_xml(
          NAME coverage_xml                 
          EXECUTABLE ctest -j ${n_cores} # Executable in PROJECT_BINARY_DIR
          EXCLUDE   "ExternalLib/*"
                    "build/*"
                    "Documents/*"
  )
  setup_target_for_coverage_gcovr_html(
          NAME coverage_html                 
          EXECUTABLE ctest -j ${n_cores} # Executable in PROJECT_BINARY_DIR
          EXCLUDE   "ExternalLib/*"
                    "build/*"
                    "Documents/*"
  )
  
  endif()
endif()

# disable all warnings for our test suite
if(MSVC)
  target_compile_options(${COMMIT_TEST_SUITE}  INTERFACE /w)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(${COMMIT_TEST_SUITE}  INTERFACE -w)
else()
  target_compile_options(${COMMIT_TEST_SUITE}  INTERFACE -w)
endif()

include(GoogleTest)
gtest_discover_tests(   ${COMMIT_TEST_SUITE})