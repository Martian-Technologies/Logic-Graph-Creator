#ifndef vulkanFrame_h
#define vulkanFrame_h

#include <vulkan/vulkan.h>
#include <chrono>

struct FrameData {
	// mostly static data
	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;
	VkSemaphore swapchainSemaphore, renderSemaphore;
	VkFence renderFence;

	// dynamically changing data
	std::chrono::time_point<std::chrono::system_clock> lastStartTime;
};

void createFrameDatas(FrameData *frameDatas, unsigned int numFrames);
void destroyFrameDatas(FrameData* frameDatas, unsigned int numFrames);

#endif
