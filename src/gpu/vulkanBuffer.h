#ifndef vulkanBuffer_h
#define vulkanBuffer_h

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

struct AllocatedBuffer {
	VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
};

AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
void destroyBuffer(AllocatedBuffer& buffer);

#endif
