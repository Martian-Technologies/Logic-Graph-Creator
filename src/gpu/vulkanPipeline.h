#ifndef vulkanPipeline_h
#define vulkanPipeline_h

#include <vulkan/vulkan.h>
#include "gpu/vulkanSwapchain.h"

struct PipelineData {
	VkPipeline handle;
    VkPipelineLayout layout;
    VkRenderPass renderPass;
};
// TODO - pipeline absolutely does not own the render pass, it should be somewhere else

PipelineData createPipeline(SwapchainData& swapchain, VkShaderModule vert, VkShaderModule frag);
void destroyPipeline(PipelineData& pipeline);

#endif
