#ifndef vulkanPipeline_h
#define vulkanPipeline_h

#include <vulkan/vulkan.h>
#include "gpu/vulkanSwapchain.h"

struct PipelineData {
	VkPipeline handle;
    VkPipelineLayout layout;
    VkRenderPass renderPass;
};

PipelineData createPipeline(VkDevice device, SwapchainData& swapchain, VkShaderModule vert, VkShaderModule frag);
void destroyPipeline(VkDevice device, PipelineData& pipeline);

VkShaderModule createShaderModule(VkDevice device, std::vector<char> byteCode);
void destroyShaderModule(VkDevice device, VkShaderModule shader);

#endif
