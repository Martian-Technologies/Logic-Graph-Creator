#ifndef vulkanShader_h
#define vulkanShader_h

#include <vulkan/vulkan.h>

VkShaderModule createShaderModule(std::vector<char> byteCode);
void destroyShaderModule(VkShaderModule shader);

#endif
