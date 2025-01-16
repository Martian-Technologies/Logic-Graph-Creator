#include "vulkanPipeline.h"

PipelineData createPipeline(VkDevice device, VkShaderModule frag, VkShaderModule vert) {
	PipelineData pipeline;

	return pipeline;
}

void destroyPipeline(VkDevice device, PipelineData& pipeline) {
	
}

VkShaderModule createShaderModule(VkDevice device, std::vector<char> byteCode) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = byteCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(byteCode.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void destroyShaderModule(VkDevice device, VkShaderModule shader) {
	vkDestroyShaderModule(device, shader, nullptr);
}
