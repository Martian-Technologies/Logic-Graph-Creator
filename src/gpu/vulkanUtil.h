#ifndef vulkanUtil_h
#define vulkanUtil_h

#include <vulkan/vulkan.h>

VkShaderModule createShaderModule(std::vector<char> byteCode);
void destroyShaderModule(VkShaderModule shader);

#endif
