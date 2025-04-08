#include "vulkanFrame.h"

#include "gpu/vulkanInstance.h"

VulkanFrameData::VulkanFrameData() {
	// command pool
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = VulkanInstance::get().getGraphicsQueue().index;
	vkCreateCommandPool(VulkanInstance::get().getDevice(), &commandPoolInfo, nullptr, &commandPool);

	// allocate the default command buffer that we will use for rendering
	VkCommandBufferAllocateInfo commandBufferInfo = {};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferInfo.pNext = nullptr;
	commandBufferInfo.commandPool = commandPool;
	commandBufferInfo.commandBufferCount = 1;
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkAllocateCommandBuffers(VulkanInstance::get().getDevice(), &commandBufferInfo, &mainCommandBuffer);

	// sync structures
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = nullptr;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreInfo.pNext = nullptr;
	semaphoreInfo.flags = 0;
	vkCreateFence(VulkanInstance::get().getDevice(), &fenceInfo, nullptr, &renderFence);
	vkCreateSemaphore(VulkanInstance::get().getDevice(), &semaphoreInfo, nullptr, &swapchainSemaphore);
	vkCreateSemaphore(VulkanInstance::get().getDevice(), &semaphoreInfo, nullptr, &renderSemaphore);

	// descriptor and buffers
	viewDataBuffer = createBuffer(sizeof(ViewData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO);
}

VulkanFrameData::~VulkanFrameData() {
	destroyBuffer(viewDataBuffer);
	
	vkDestroyCommandPool(VulkanInstance::get().getDevice(), commandPool, nullptr);

	vkDestroyFence(VulkanInstance::get().getDevice(), renderFence, nullptr);
	vkDestroySemaphore(VulkanInstance::get().getDevice(), renderSemaphore, nullptr);
	vkDestroySemaphore(VulkanInstance::get().getDevice(), swapchainSemaphore, nullptr);
}

void VulkanFrameData::start() {
	// reset render fence (we are actually rendering this frame)
	vkResetFences(VulkanInstance::get().getDevice(), 1, &renderFence);
	
	// update start time
	lastStartTime = std::chrono::system_clock::now();
}

float VulkanFrameData::waitAndComplete() {
	// wait until current frame has finished rendering
	vkWaitForFences(VulkanInstance::get().getDevice(), 1, &renderFence, VK_TRUE, UINT64_MAX);

	// update frame time with newest frame completion
	float time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastStartTime).count();
		
	// clear used allocations
	chunkAllocations.clear();
	rmlAllocations.clear();

	return time;
}
