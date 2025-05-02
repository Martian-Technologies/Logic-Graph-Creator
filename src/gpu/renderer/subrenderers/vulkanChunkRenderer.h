#ifndef vulkanChunkRenderer_h
#define vulkanChunkRenderer_h

#include "gpu/renderer/frameManager.h"
#include "gpu/renderer/vulkanPipeline.h"
#include "gpu/vulkanDescriptor.h"
#include "gpu/vulkanImage.h"
#include "vulkanChunker.h"

struct ChunkPushConstants {
	glm::mat4 mvp;
};

class VulkanChunkRenderer {
public:
	VulkanChunkRenderer(VkRenderPass& renderPass);
	~VulkanChunkRenderer();
	
	void render(Frame& frame, const glm::mat4& viewMatrix, const std::vector<std::shared_ptr<VulkanChunkAllocation>>& chunks);

private:
	std::unique_ptr<Pipeline> blockPipeline = nullptr;
	std::unique_ptr<Pipeline> wirePipeline = nullptr;

	// descriptors and textures
	DescriptorAllocator descriptorAllocator;
	// block texture
	VkDescriptorSetLayout blockTextureDescriptorSetLayout;
	VkDescriptorSet blockTextureDescriptor;
	VkSampler blockTextureSampler;
	AllocatedImage blockTexture;
	// state buffer
	VkDescriptorSetLayout stateBufferDescriptorSetLayout;
};

#endif
