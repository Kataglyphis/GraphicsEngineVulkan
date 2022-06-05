#pragma once

#include <string> // std::string
#include <iostream> // std::cout
#include <sstream> // std::stringstream

#include "host_device_shared_vars.h"

// Error checking on vulkan function calls
#define ASSERT_VULKAN(val, error_string)                                                                                                                       \
  if (val != VK_SUCCESS) {                                                                                                                                     \
    throw std::runtime_error(error_string);                                                                                                                    \
  }

#define NOT_YET_IMPLEMENTED throw std::runtime_error("Not yet implemented!");

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif
