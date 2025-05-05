#include "vulkanShader.h"

#include "gpu/vulkanInstance.h"

VkShaderModule createShaderModule(std::vector<char> byteCode) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = byteCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(byteCode.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(VulkanInstance::get().getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throwFatalError("failed to create vulkan shader module!");
	}

	return shaderModule;
}

void destroyShaderModule(VkShaderModule shader) {
	vkDestroyShaderModule(VulkanInstance::get().getDevice(), shader, nullptr);
}
