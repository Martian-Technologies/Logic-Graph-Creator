#include "vulkanFrame.h"
#include "gpu/vulkanManager.h"

void createFrameDatas(FrameData *frameDatas, unsigned int numFrames) {

	// create shared command pool info
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = Vulkan::getQueueFamilies().graphicsFamily.value().index;

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
		vkCreateCommandPool(Vulkan::getDevice(), &commandPoolInfo, nullptr, &frameDatas[i].commandPool);

		// allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo commandBufferInfo = {};
		commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferInfo.pNext = nullptr;
		commandBufferInfo.commandPool = frameDatas[i].commandPool;
		commandBufferInfo.commandBufferCount = 1;
		commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		vkAllocateCommandBuffers(Vulkan::getDevice(), &commandBufferInfo, &frameDatas[i].mainCommandBuffer);

		// allocate sync structures
		vkCreateFence(Vulkan::getDevice(), &fenceInfo, nullptr, &frameDatas[i].renderFence);

		vkCreateSemaphore(Vulkan::getDevice(), &semaphoreInfo, nullptr, &frameDatas[i].swapchainSemaphore);
		vkCreateSemaphore(Vulkan::getDevice(), &semaphoreInfo, nullptr, &frameDatas[i].renderSemaphore);
	}
}

void destroyFrameDatas(FrameData* frameDatas, unsigned int numFrames) {
	for (int i = 0; i < numFrames; i++) {
		vkDestroyCommandPool(Vulkan::getDevice(), frameDatas[i].commandPool, nullptr);

		vkDestroyFence(Vulkan::getDevice(), frameDatas[i].renderFence, nullptr);
		vkDestroySemaphore(Vulkan::getDevice(), frameDatas[i].renderSemaphore, nullptr);
		vkDestroySemaphore(Vulkan::getDevice(), frameDatas[i].swapchainSemaphore, nullptr);
	}
}
