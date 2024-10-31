# we depend on vulkan 
find_package(Vulkan REQUIRED)
# configure vulkan version
set(VULKAN_VERSION_MAJOR 1)
set(VULKAN_VERSION_MINOR 3)
find_package(Threads REQUIRED)
