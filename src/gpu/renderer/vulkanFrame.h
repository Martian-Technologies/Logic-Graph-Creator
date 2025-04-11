#ifndef vulkanFrame_h
#define vulkanFrame_h

#include <vulkan/vulkan.h>
#include <chrono>
#include <glm/ext/matrix_float4x4.hpp>

#include "gpu/vulkanBuffer.h"

struct VulkanChunkAllocation;
struct RmlGeometryAllocation;
struct RmlTexture;

class VulkanFrameData {
public:
	VulkanFrameData();
	~VulkanFrameData();

	void start();
	float waitAndComplete();

	inline AllocatedBuffer& getViewDataBuffer() { return viewDataBuffer; }
	inline VkDescriptorSet& getViewDataDescriptorSet() { return viewDataDescriptorSet; }

	inline VkCommandPool& getCommandPool() { return commandPool; };
	inline VkCommandBuffer& getMainCommandBuffer() { return mainCommandBuffer; };
	inline VkSemaphore& getSwapchainSemaphore() { return swapchainSemaphore; };
	inline VkSemaphore& getRenderSemaphore() { return renderSemaphore; }
	inline VkFence& getRenderFence() { return renderFence; };
	
	inline std::vector<std::shared_ptr<VulkanChunkAllocation>>& getChunkAllocations() { return chunkAllocations; }
	inline std::vector<std::shared_ptr<RmlGeometryAllocation>>& getRmlAllocations() { return rmlAllocations; }
	inline std::vector<std::shared_ptr<RmlGeometryAllocation>>& getRmlTextures() { return rmlTextures; }
	
private:
	// mostly static data
	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;
	VkSemaphore swapchainSemaphore, renderSemaphore;
	VkFence renderFence;

	AllocatedBuffer viewDataBuffer; // buffer for storing view matrix which is pointed to by descriptor
	VkDescriptorSet viewDataDescriptorSet;

	// dynamically changing data
	std::chrono::time_point<std::chrono::system_clock> lastStartTime;
	std::vector<std::shared_ptr<VulkanChunkAllocation>> chunkAllocations;
	std::vector<std::shared_ptr<RmlGeometryAllocation>> rmlAllocations;
	std::vector<std::shared_ptr<RmlGeometryAllocation>> rmlTextures;
};

#endif
