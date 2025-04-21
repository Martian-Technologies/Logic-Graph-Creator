#ifndef vulkanPipeline_h
#define vulkanPipeline_h

#include <vulkan/vulkan.h>
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
	Pipeline(const PipelineInformation& info);
	~Pipeline();

	inline VkPipeline getHandle() { return handle; }
	inline VkPipelineLayout getLayout() { return layout; }
	
private:
	VkPipeline handle;
    VkPipelineLayout layout;
};

#endif
