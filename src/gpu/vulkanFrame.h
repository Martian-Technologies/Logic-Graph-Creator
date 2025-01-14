#ifndef vulkanFrame_h
#define vulkanFrame_h

#include <vulkan/vulkan.h>

struct FrameData {
	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;
	VkSemaphore swapchainSemaphore, renderSemaphore;
	VkFence renderFence;        
};

void createFrameDatas(VkDevice device, FrameData *frameDatas, unsigned int numFrames, unsigned int graphicsQueueIndex);
void destroyFrameDatas(VkDevice device, FrameData* frameDatas, unsigned int numFrames);

#endif
