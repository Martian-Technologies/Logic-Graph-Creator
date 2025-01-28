#ifndef vulkanFrame_h
#define vulkanFrame_h

#include <vulkan/vulkan.h>

struct FrameData {
	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;
	VkSemaphore swapchainSemaphore, renderSemaphore;
	VkFence renderFence;        
};

void createFrameDatas(FrameData *frameDatas, unsigned int numFrames);
void destroyFrameDatas(FrameData* frameDatas, unsigned int numFrames);

#endif
