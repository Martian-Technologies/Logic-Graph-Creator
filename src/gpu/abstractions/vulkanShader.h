#ifndef vulkanShader_h
#define vulkanShader_h

#include "gpu/vulkanDevice.h"

VkShaderModule createShaderModule(VkDevice device, std::vector<char> byteCode);
void destroyShaderModule(VkDevice device, VkShaderModule shader);

#endif
