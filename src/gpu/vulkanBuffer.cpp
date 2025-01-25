#include "vulkanBuffer.h"

#include "vulkanManager.h"

AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
	// allocate buffer
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = allocSize;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = memoryUsage;
	vmaallocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	AllocatedBuffer newBuffer;

	// allocate the buffer
	vmaCreateBuffer(Vulkan::getAllocator(), &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation, &newBuffer.info);
	return newBuffer;
}

void destroyBuffer(AllocatedBuffer& buffer) {
	vmaDestroyBuffer(Vulkan::getAllocator(), buffer.buffer, buffer.allocation);
}
