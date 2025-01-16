#ifndef vulkanPipeline_h
#define vulkanPipeline_h

#include <vulkan/vulkan.h>

struct PipelineData {
	VkPipeline handle;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
};

PipelineData createPipeline(VkDevice device, VkShaderModule frag, VkShaderModule vert);
void destroyPipeline(VkDevice device, PipelineData& pipeline);

VkShaderModule createShaderModule(VkDevice device, std::vector<char> byteCode);
void destroyShaderModule(VkDevice device, VkShaderModule shader);

#endif
