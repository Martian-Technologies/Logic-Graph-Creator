#include "frameManager.h"
#include <thread>

void Frame::init(VulkanDevice* device) {
	this->device = device;
	
	// command pool
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = device->getGraphicsQueueIndex();
	vkCreateCommandPool(device->getDevice(), &commandPoolInfo, nullptr, &commandPool);
	// allocate the default command buffer that we will use for rendering
	VkCommandBufferAllocateInfo commandBufferInfo = {};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferInfo.pNext = nullptr;
	commandBufferInfo.commandPool = commandPool;
	commandBufferInfo.commandBufferCount = 1;
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkAllocateCommandBuffers(device->getDevice(), &commandBufferInfo, &mainCommandBuffer);

	// sync structures
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = nullptr;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreInfo.pNext = nullptr;
	semaphoreInfo.flags = 0;
	vkCreateFence(device->getDevice(), &fenceInfo, nullptr, &renderFence);
	vkCreateSemaphore(device->getDevice(), &semaphoreInfo, nullptr, &swapchainSemaphore);
}

void Frame::cleanup() {
	vkDestroyCommandPool(device->getDevice(), commandPool, nullptr);

	vkDestroyFence(device->getDevice(), renderFence, nullptr);
	vkDestroySemaphore(device->getDevice(), swapchainSemaphore, nullptr);
}

void FrameManager::init(VulkanDevice* device) {
	for (Frame& frame : frames) {
		frame.init(device);
	}
}

void FrameManager::cleanup() {
	for (Frame& frame : frames) {
		frame.cleanup();
	}
}

void FrameManager::incrementFrame() {
	++frameNumber;
	frameIndex = frameNumber % frames.size();
}

const float minFrameTime = 6.9f;
const bool frameLimit = true;

float FrameManager::waitForCurrentFrameCompletion() {
	// wait until current frame has finished rendering
	vkWaitForFences(frames[frameIndex].device->getDevice(), 1, &frames[frameIndex].renderFence, VK_TRUE, UINT64_MAX);

	// update frame time with newest frame completion
	auto time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - frames[frameIndex].lastStartTime).count() / 1000.0f;

	// if we are limiting frame times and under the limit, wait
	float timeRemaining = minFrameTime - time;
	if (frameLimit && timeRemaining > 0.0f) {
		std::this_thread::sleep_for(std::chrono::microseconds(long(timeRemaining * 1000.0f)));
	}

	// clear used allocations
	frames[frameIndex].lifetime.flush();

	return time + std::max(0.0f, timeRemaining);
}

void FrameManager::startCurrentFrame() {
	// reset render fence (we are actually rendering this frame)
	vkResetFences(frames[frameIndex].device->getDevice(), 1, &frames[frameIndex].renderFence);
	
	// update start time
	frames[frameIndex].lastStartTime = std::chrono::system_clock::now();
}

