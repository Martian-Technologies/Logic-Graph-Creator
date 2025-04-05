#ifndef vulkanPipeline_h
#define vulkanPipeline_h

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

struct PipelineData {
	VkPipeline handle;
    VkPipelineLayout layout;
};

struct ViewPushConstants {
	glm::mat4 mvp;
};

struct RmlPushConstants {
	glm::vec2 translation;
};

PipelineData createPipeline(VkShaderModule vert, VkShaderModule frag, const std::vector<VkVertexInputBindingDescription>& bindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions, size_t pushConstantsSize, VkRenderPass renderPass);
void destroyPipeline(PipelineData& pipeline);

#endif
