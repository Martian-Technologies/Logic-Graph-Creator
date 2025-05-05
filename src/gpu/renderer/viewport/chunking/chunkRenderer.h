#ifndef chunkRenderer_h
#define chunkRenderer_h

#include "gpu/renderer/frameManager.h"
#include "gpu/abstractions/vulkanPipeline.h"
#include "gpu/abstractions/vulkanDescriptor.h"
#include "gpu/abstractions/vulkanImage.h"
#include "vulkanChunker.h"

struct ChunkPushConstants {
	glm::mat4 mvp;
};

class ChunkRenderer {
public:
	ChunkRenderer(VkRenderPass& renderPass);
	~ChunkRenderer();
	
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
