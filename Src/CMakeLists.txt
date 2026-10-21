function(_myproject_configure_windows_rust_crate rust_target)
  target_link_libraries(
    ${rust_target}
    INTERFACE kernel32
              ntdll
              userenv
              ws2_32
              dbghelp)

  set(_RUST_WINDOWS_DEBUG_CXXFLAGS "/MDd /D_DEBUG /D_ITERATOR_DEBUG_LEVEL=2")
  set(_RUST_WINDOWS_DEBUG_CFLAGS "/MDd /D_DEBUG")

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
     AND MSVC
     AND myproject_ENABLE_SANITIZER_ADDRESS)
    set(_RUST_WINDOWS_DEBUG_CXXFLAGS "/MD /D_ITERATOR_DEBUG_LEVEL=0")
    set(_RUST_WINDOWS_DEBUG_CFLAGS "/MD")
  endif()

  set(_RUST_WINDOWS_CXXFLAGS
      "/EHsc $<$<CONFIG:Debug>:${_RUST_WINDOWS_DEBUG_CXXFLAGS}>$<$<NOT:$<CONFIG:Debug>>:/MD /DNDEBUG /D_ITERATOR_DEBUG_LEVEL=0>"
  )
  set(_RUST_WINDOWS_CFLAGS "$<$<CONFIG:Debug>:${_RUST_WINDOWS_DEBUG_CFLAGS}>$<$<NOT:$<CONFIG:Debug>>:/MD /DNDEBUG>")

  corrosion_set_env_vars(${rust_target} "CXXFLAGS=${_RUST_WINDOWS_CXXFLAGS}" "CFLAGS=${_RUST_WINDOWS_CFLAGS}")
endfunction()

function(_myproject_configure_native_rust_crate rust_target)
  set(RUST_CC "${CMAKE_C_COMPILER}")
  set(RUST_CXX "${CMAKE_CXX_COMPILER}")
  if(NOT RUST_CC)
    set(RUST_CC "cc")
  endif()
  if(NOT RUST_CXX)
    set(RUST_CXX "c++")
  endif()

  # No --gcc-toolchain: cc-rs targets the host triple, so the image's <triple>-clang++.cfg selects its GCC.
  set(RUST_CXXFLAGS "-fexceptions")
  if(DEFINED ENV{CXXFLAGS})
    set(RUST_CXXFLAGS "$ENV{CXXFLAGS} ${RUST_CXXFLAGS}")
  endif()

  corrosion_set_env_vars(
    ${rust_target}
    CC=${RUST_CC}
    CXX=${RUST_CXX}
    "CXXFLAGS=${RUST_CXXFLAGS}")

  message(STATUS "Configured native corrosion: CC=${RUST_CC} CXX=${RUST_CXX} CXXFLAGS=${RUST_CXXFLAGS}")
endfunction()

# The bridge crate alone: OxidANT's root package links GStreamer and the ORT loader, which the engine never calls.
corrosion_import_crate(
  MANIFEST_PATH
  ../third_party/OxidANT/Cargo.toml
  CRATE_TYPES
  staticlib
  CRATES
  kataglyphis_cxx_bridge)

if(WIN32)
  _myproject_configure_windows_rust_crate(kataglyphis_cxx_bridge)
else()
  _myproject_configure_native_rust_crate(kataglyphis_cxx_bridge)
endif()

corrosion_add_cxxbridge(
  oxidant_bridge
  CRATE
  kataglyphis_cxx_bridge
  FILES
  lib.rs)

add_subdirectory(GraphicsEngineVulkan)
# Off by default: kompute's headers throw, so the playground cannot use the no-exceptions options.
if(KATAGLYPHIS_BUILD_KOMPUTE_PLAYGROUND)
  add_subdirectory(KomputePlayground)
endif()
