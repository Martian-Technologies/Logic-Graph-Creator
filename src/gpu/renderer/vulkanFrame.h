#ifndef vulkanFrame_h
#define vulkanFrame_h

#include <vulkan/vulkan.h>
#include <chrono>

#include "gpu/renderer/vulkanChunker.h"

struct FrameData {
	// mostly static data
	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;
	VkSemaphore swapchainSemaphore, renderSemaphore;
	VkFence renderFence;

	// dynamically changing data
	std::chrono::time_point<std::chrono::system_clock> lastStartTime;
	std::vector<std::shared_ptr<VulkanChunkAllocation>> usingChunkAllocations;
};

void createFrameDatas(FrameData *frameDatas, unsigned int numFrames);
void destroyFrameDatas(FrameData* frameDatas, unsigned int numFrames);

#endif
