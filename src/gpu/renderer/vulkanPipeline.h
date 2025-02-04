#ifndef vulkanPipeline_h
#define vulkanPipeline_h

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "vulkanSwapchain.h"

struct PipelineData {
	VkPipeline handle;
    VkPipelineLayout layout;
    VkRenderPass renderPass;
};

struct VertexPushConstants {
	alignas(16) glm::mat4 mvp;
};

// TODO - pipeline absolutely does not own the render pass, it should be somewhere else

PipelineData createPipeline(SwapchainData& swapchain, VkShaderModule vert, VkShaderModule frag);
void destroyPipeline(PipelineData& pipeline);

#endif
