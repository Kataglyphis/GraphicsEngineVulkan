# we depend on vulkan
find_package(Vulkan REQUIRED)
# configure vulkan version
set(VULKAN_VERSION_MAJOR 1)
set(VULKAN_VERSION_MINOR 3)
find_package(Threads REQUIRED)

# we depend on OpenGL
find_package( OpenGL REQUIRED COMPONENTS OpenGL)
# configure OpenGL version
set(OPENGL_VERSION_MAJOR 4)
set(OPENGL_VERSION_MINOR 6)
set(OpenGL_GL_PREFERENCE GLVND)
