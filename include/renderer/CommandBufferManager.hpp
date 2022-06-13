#pragma once
#include <vulkan/vulkan.h>

#include "Utilities.hpp"

class CommandBufferManager {
 public:
  CommandBufferManager();

  VkCommandBuffer beginCommandBuffer(VkDevice device,
                                     VkCommandPool command_pool);
  void endAndSubmitCommandBuffer(VkDevice device, VkCommandPool command_pool,
                                 VkQueue queue,
                                 VkCommandBuffer& command_buffer);

  ~CommandBufferManager();

 private:
};
