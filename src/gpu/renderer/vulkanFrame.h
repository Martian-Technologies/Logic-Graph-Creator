#ifndef vulkanFrame_h
#define vulkanFrame_h

#include <vulkan/vulkan.h>
#include <chrono>

#include "gpu/renderer/rmlRenderer.h"
#include "gpu/renderer/vulkanChunker.h"

class VulkanFrameData {
public:
	VulkanFrameData();
	~VulkanFrameData();

	void start();
	float waitAndComplete();

	inline VkCommandPool& getCommandPool() { return commandPool; };
	inline VkCommandBuffer& getMainCommandBuffer() { return mainCommandBuffer; };
	inline VkSemaphore& getSwapchainSemaphore() { return swapchainSemaphore; };
	inline VkSemaphore& getRenderSemaphore() { return renderSemaphore; }
	inline VkFence& getRenderFence() { return renderFence; };
	inline std::vector<std::shared_ptr<VulkanChunkAllocation>>& getChunkAllocations() { return chunkAllocations; }
	inline std::vector<std::shared_ptr<RmlGeometryAllocation>>& getRmlAllocations() { return rmlAllocations; }
	
private:
	// mostly static data
	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;
	VkSemaphore swapchainSemaphore, renderSemaphore;
	VkFence renderFence;

	// dynamically changing data
	std::chrono::time_point<std::chrono::system_clock> lastStartTime;
	std::vector<std::shared_ptr<VulkanChunkAllocation>> chunkAllocations;
	std::vector<std::shared_ptr<RmlGeometryAllocation>> rmlAllocations;
};

#endif
