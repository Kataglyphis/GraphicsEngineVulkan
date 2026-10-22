# The bridge crate alone: OxidANT's root package links GStreamer and the ORT loader, which the engine never calls.
corrosion_import_crate(
  MANIFEST_PATH
  ../third_party/OxidANT/Cargo.toml
  CRATE_TYPES
  staticlib
  CRATES
  kataglyphis_cxx_bridge)

include(CorrosionToolchain)
kataglyphis_configure_corrosion_target(kataglyphis_cxx_bridge ADDRESS_SANITIZER ${myproject_ENABLE_SANITIZER_ADDRESS})

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
