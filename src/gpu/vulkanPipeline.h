#ifndef vulkanPipeline_h
#define vulkanPipeline_h

#include <vulkan/vulkan.h>
#include "gpu/vulkanSwapchain.h"

struct PipelineData {
	VkPipeline handle;
    VkPipelineLayout layout;
    VkRenderPass renderPass;
};

PipelineData createPipeline(SwapchainData& swapchain, VkShaderModule vert, VkShaderModule frag);
void destroyPipeline(PipelineData& pipeline);

#endif
