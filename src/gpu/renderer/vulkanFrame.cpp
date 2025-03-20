#include "vulkanFrame.h"
#include "gpu/vulkanManager.h"

VulkanFrameData::VulkanFrameData() {
	// command pool
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = Vulkan::getQueueFamilies().graphicsFamily.value().index;
	vkCreateCommandPool(Vulkan::getDevice(), &commandPoolInfo, nullptr, &commandPool);

	// allocate the default command buffer that we will use for rendering
	VkCommandBufferAllocateInfo commandBufferInfo = {};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferInfo.pNext = nullptr;
	commandBufferInfo.commandPool = commandPool;
	commandBufferInfo.commandBufferCount = 1;
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkAllocateCommandBuffers(Vulkan::getDevice(), &commandBufferInfo, &mainCommandBuffer);

	// sync structures
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = nullptr;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreInfo.pNext = nullptr;
	semaphoreInfo.flags = 0;
	vkCreateFence(Vulkan::getDevice(), &fenceInfo, nullptr, &renderFence);
	vkCreateSemaphore(Vulkan::getDevice(), &semaphoreInfo, nullptr, &swapchainSemaphore);
	vkCreateSemaphore(Vulkan::getDevice(), &semaphoreInfo, nullptr, &renderSemaphore);
}

VulkanFrameData::~VulkanFrameData() {
	vkDestroyCommandPool(Vulkan::getDevice(), commandPool, nullptr);

	vkDestroyFence(Vulkan::getDevice(), renderFence, nullptr);
	vkDestroySemaphore(Vulkan::getDevice(), renderSemaphore, nullptr);
	vkDestroySemaphore(Vulkan::getDevice(), swapchainSemaphore, nullptr);
}

void VulkanFrameData::start() {
	// reset render fence (we are actually rendering this frame)
	vkResetFences(Vulkan::getDevice(), 1, &renderFence);
	
	// update start time
	lastStartTime = std::chrono::system_clock::now();
}

float VulkanFrameData::waitAndComplete() {
	// wait until current frame has finished rendering
	vkWaitForFences(Vulkan::getDevice(), 1, &renderFence, VK_TRUE, UINT64_MAX);

	// update frame time with newest frame completion
	float time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastStartTime).count();
		
	// clear used chunks
	chunkAllocations.clear();

	return time;
}
