#ifndef vulkanPipeline_h
#define vulkanPipeline_h

#include "gpu/vulkanDevice.h"
#include <glm/glm.hpp>

struct PushConstantDescription {
	size_t size;
	size_t offset;
	VkShaderStageFlags stage;
};

struct PipelineInformation {
	VkRenderPass renderPass;
	VkShaderModule vertShader, fragShader;
	std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
	VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;

	std::vector<PushConstantDescription> pushConstants;
	std::vector<VkDescriptorSetLayout> descriptorSets;
};

class Pipeline {
public:
	void init(VulkanDevice* device, const PipelineInformation& info);
	void cleanup();

	inline VkPipeline getHandle() { return handle; }
	inline VkPipelineLayout getLayout() { return layout; }
	
private:
	VkPipeline handle;
    VkPipelineLayout layout;

	VulkanDevice* device;
};

#endif
