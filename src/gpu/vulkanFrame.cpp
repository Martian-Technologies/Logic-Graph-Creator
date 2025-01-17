#include "vulkanFrame.h"

void createFrameDatas(VkDevice device, FrameData *frameDatas, unsigned int numFrames, unsigned int graphicsQueueIndex) {

	// create shared command pool info
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = graphicsQueueIndex;

	// shared sync structure create infos
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = nullptr;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreInfo.pNext = nullptr;
	semaphoreInfo.flags = 0;
	
	for (int i = 0; i < numFrames; i++) {
		// allocate command pool
		vkCreateCommandPool(device, &commandPoolInfo, nullptr, &frameDatas[i].commandPool);

		// allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo commandBufferInfo = {};
		commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferInfo.pNext = nullptr;
		commandBufferInfo.commandPool = frameDatas[i].commandPool;
		commandBufferInfo.commandBufferCount = 1;
		commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		vkAllocateCommandBuffers(device, &commandBufferInfo, &frameDatas[i].mainCommandBuffer);

		// allocate sync structures
		vkCreateFence(device, &fenceInfo, nullptr, &frameDatas[i].renderFence);

		vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frameDatas[i].swapchainSemaphore);
		vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frameDatas[i].renderSemaphore);
	}
}

void destroyFrameDatas(VkDevice device, FrameData* frameDatas, unsigned int numFrames) {
	for (int i = 0; i < numFrames; i++) {
		vkDestroyCommandPool(device, frameDatas[i].commandPool, nullptr);

		vkDestroyFence(device, frameDatas[i].renderFence, nullptr);
		vkDestroySemaphore(device, frameDatas[i].renderSemaphore, nullptr);
		vkDestroySemaphore(device, frameDatas[i].swapchainSemaphore, nullptr);
	}
}
