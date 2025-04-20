#ifndef vulkanChunkRenderer_h
#define vulkanChunkRenderer_h

#include "gpu/renderer/vulkanFrame.h"
#include "gpu/renderer/vulkanPipeline.h"
#include "backend/circuit/circuit.h"
#include "gpu/vulkanDescriptor.h"
#include "gpu/vulkanImage.h"
#include "vulkanChunker.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct ViewPushConstants {
	glm::mat4 mvp;
};

struct BlockVertex {
	glm::vec2 pos;
	glm::vec2 tex;

	inline static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(BlockVertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }

	inline static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(BlockVertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(BlockVertex, tex);

		return attributeDescriptions;
	}
};

struct WireVertex {
	glm::vec2 pos;

	inline static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(WireVertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }

	inline static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(WireVertex, pos);

		return attributeDescriptions;
	}
};

class VulkanChunkRenderer {
public:
	VulkanChunkRenderer(VkRenderPass& renderPass);
	~VulkanChunkRenderer();
	
	void render(VulkanFrameData& frame, VkViewport& viewport, const glm::mat4& viewMatrix, const std::vector<std::shared_ptr<VulkanChunkAllocation>>& chunks);

private:
	std::unique_ptr<Pipeline> blockPipeline = nullptr;
	std::unique_ptr<Pipeline> wirePipeline = nullptr;

	// descriptors and textures
	DescriptorAllocator descriptorAllocator;
	VkDescriptorSetLayout blockTextureDescriptorSetLayout;
	VkSampler blockTextureSampler;
	VkDescriptorSet blockTextureDescriptor;
	AllocatedImage blockTexture;
};

#endif
