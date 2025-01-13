#include "vulkanFrame.h"

void createFrameDatas(VkDevice device, FrameData *frameDatas, unsigned int numFrames, unsigned int graphicsQueueIndex) {
	VkCommandPoolCreateInfo commandPoolInfo =  {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = graphicsQueueIndex;
	
	for (int i = 0; i < numFrames; i++) {
		vkCreateCommandPool(device, &commandPoolInfo, nullptr, &frameDatas[i]._commandPool);

		// allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo cmdAllocInfo = {};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.pNext = nullptr;
		cmdAllocInfo.commandPool = frameDatas[i]._commandPool;
		cmdAllocInfo.commandBufferCount = 1;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		vkAllocateCommandBuffers(device, &cmdAllocInfo, &frameDatas[i]._mainCommandBuffer);
	}
}

void destroyFrameDatas(VkDevice device, FrameData* frameDatas, unsigned int numFrames) {
	for (int i = 0; i < numFrames; i++) {
		vkDestroyCommandPool(device, frameDatas[i]._commandPool, nullptr);
	}
}
