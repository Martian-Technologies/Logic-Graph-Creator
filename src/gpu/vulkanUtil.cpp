#include "vulkanUtil.h"

#include "gpu/vulkanManager.h"

VkShaderModule createShaderModule(std::vector<char> byteCode) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = byteCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(byteCode.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(Vulkan::Device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void destroyShaderModule(VkShaderModule shader) {
	vkDestroyShaderModule(Vulkan::Device(), shader, nullptr);
}
