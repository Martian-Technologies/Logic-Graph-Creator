#include "vulkanBuffer.h"

#include "vulkanInstance.h"

AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaAllocationCreateFlags flags) {
	// allocate buffer
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = allocSize;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo vmaAllocInfo = {};
	vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	vmaAllocInfo.flags = flags;
	AllocatedBuffer newBuffer;

	// allocate the buffer
	VkResult result = vmaCreateBuffer(VulkanInstance::get().getAllocator(), &bufferInfo, &vmaAllocInfo, &newBuffer.buffer, &newBuffer.allocation, &newBuffer.info);
	if(result != VK_SUCCESS) {
		throwFatalError("failed to create vulkan buffer");
	}
	return newBuffer;
}

void destroyBuffer(AllocatedBuffer& buffer) {
	vmaDestroyBuffer(VulkanInstance::get().getAllocator(), buffer.buffer, buffer.allocation);
}
