#ifndef vulkanPipeline_h
#define vulkanPipeline_h

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

struct PipelineData {
	VkPipeline handle;
    VkPipelineLayout layout;
};

struct ViewPushConstants {
	alignas(16) glm::mat4 mvp;
};

// TODO - pipeline absolutely does not own the render pass, it should be somewhere else

PipelineData createPipeline(VkShaderModule vert, VkShaderModule frag, std::vector<VkVertexInputBindingDescription> bindingDescriptions, std::vector<VkVertexInputAttributeDescription> attributeDescriptions, VkRenderPass renderPass);
void destroyPipeline(PipelineData& pipeline);

#endif
