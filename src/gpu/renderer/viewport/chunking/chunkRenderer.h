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
	void init(VulkanDevice* device, VkRenderPass& renderPass);
	void cleanup();
	
	void render(Frame& frame, const glm::mat4& viewMatrix, const std::vector<std::shared_ptr<VulkanChunkAllocation>>& chunks);

private:
	Pipeline blockPipeline;
	Pipeline wirePipeline;

	DescriptorAllocator descriptorAllocator;

	// block texture and descriptor
	VkDescriptorSetLayout blockTextureDescriptorSetLayout;
	VkDescriptorSet blockTextureDescriptor;
	VkSampler blockTextureSampler;
	AllocatedImage blockTexture;
	
	// state buffer descriptor layout
	VkDescriptorSetLayout stateBufferDescriptorSetLayout;

	// refs
	VulkanDevice* device;
};

#endif
