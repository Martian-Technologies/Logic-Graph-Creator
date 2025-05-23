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

PipelineData createPipeline(VkShaderModule vert, VkShaderModule frag, std::vector<VkVertexInputBindingDescription> bindingDescriptions, std::vector<VkVertexInputAttributeDescription> attributeDescriptions, VkRenderPass renderPass);
void destroyPipeline(PipelineData& pipeline);

#endif
