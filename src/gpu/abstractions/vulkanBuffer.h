#ifndef vulkanBuffer_h
#define vulkanBuffer_h

#include "gpu/vulkanDevice.h"

struct AllocatedBuffer {
	VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;

	VulkanDevice* device;
};

AllocatedBuffer createBuffer(VulkanDevice* device, size_t allocSize, VkBufferUsageFlags usage, VmaAllocationCreateFlags flags);
void destroyBuffer(AllocatedBuffer& buffer);

#endif
