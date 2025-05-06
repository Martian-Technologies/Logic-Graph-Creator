#include "vulkanShader.h"

VkShaderModule createShaderModule(VkDevice device, std::vector<char> byteCode) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = byteCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(byteCode.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throwFatalError("failed to create vulkan shader module!");
	}

	return shaderModule;
}

void destroyShaderModule(VkDevice device, VkShaderModule shader) {
	vkDestroyShaderModule(device, shader, nullptr);
}
