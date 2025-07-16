#include "nBuffer.h"

void NBuffer::init(VulkanDevice* device, size_t allocSize, VkBufferUsageFlags usage, VmaAllocationCreateFlags flags) {
	this->device = device;

	// create buffers and infos
	for (uint32_t i = 0; i < buffers.size(); ++i){
		buffers[i] = createBuffer(device, allocSize, usage, flags);
		bufferInfos[i] = {
			.buffer = buffers[i].buffer,
			.offset = 0,
			.range = allocSize
		};
	}
}

void NBuffer::cleanup() {
	for (auto& buffer : buffers) {
		destroyBuffer(buffer);
	}
}
