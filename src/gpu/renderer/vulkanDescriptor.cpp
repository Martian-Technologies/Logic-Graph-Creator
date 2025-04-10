#include "vulkanDescriptor.h"

#include "gpu/vulkanInstance.h"

void DescriptorLayoutBuilder::addBinding(uint32_t bindingIndex, VkDescriptorType type) {
	VkDescriptorSetLayoutBinding binding{};
	binding.binding = bindingIndex;
	binding.descriptorCount = 1;
	binding.descriptorType = type;

	bindings.push_back(binding);
}

VkDescriptorSetLayout DescriptorLayoutBuilder::build(VkShaderStageFlags shaderStages, VkDescriptorSetLayoutCreateFlags flags) {
	// set shader stage for all bindings
	for (auto& binding : bindings) {
		binding.stageFlags |= shaderStages;
	}

	if (bindings.size() == 0) {
		logWarning("Attempting to create descriptor set layout with 0 bindings", "Vulkan");
	}

	// Create descriptor set layout
	VkDescriptorSetLayoutCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = bindings.size();
	info.pBindings = bindings.data();
	info.flags = flags;

	VkDescriptorSetLayout layout;
	vkCreateDescriptorSetLayout(VulkanInstance::get().getDevice(), &info, nullptr, &layout);

	return layout;
}

void DescriptorWriter::writeImage(int bindingIndex, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type) {
	// create image info
	VkDescriptorImageInfo& info = imageInfos.emplace_back(VkDescriptorImageInfo{
		.sampler = sampler,
		.imageView = image,
		.imageLayout = layout
	});

	// create write
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstBinding = bindingIndex;
	write.dstSet = VK_NULL_HANDLE; // left empty for now until the write
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pImageInfo = &info;
	writes.push_back(write);
}

void DescriptorWriter::writeBuffer(int bindingIndex, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type) {
	// create buffer info
	VkDescriptorBufferInfo& info = bufferInfos.emplace_back(VkDescriptorBufferInfo{
		.buffer = buffer,
		.offset = offset,
		.range = size
	});

	// create write
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstBinding = bindingIndex;
	write.dstSet = VK_NULL_HANDLE; // left empty for now until the write
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pBufferInfo = &info;
	writes.push_back(write);
}

void DescriptorWriter::updateSet(VkDevice device, VkDescriptorSet set) {
	for (VkWriteDescriptorSet& write : writes) {
        write.dstSet = set;
    }

    vkUpdateDescriptorSets(device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
}

// ============================= DESCRIPTOR ALLOCATOR ===============================================
DescriptorAllocator::DescriptorAllocator(uint32_t maxSets, const std::vector<PoolSizeRatio>& poolRatios) {
	// Get actual pool sizes
	std::vector<VkDescriptorPoolSize> poolSizes;
	for (const PoolSizeRatio& ratio : poolRatios) {
		poolSizes.push_back({ratio.type, uint32_t(ratio.ratio * maxSets)});
	}

	// create pool
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = 0;
	poolInfo.maxSets = maxSets;
	poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();

	vkCreateDescriptorPool(VulkanInstance::get().getDevice(), &poolInfo, nullptr, &pool);
}

DescriptorAllocator::~DescriptorAllocator() {
	vkDestroyDescriptorPool(VulkanInstance::get().getDevice(), pool, nullptr);
}

void DescriptorAllocator::clearDescriptors() {
	vkResetDescriptorPool(VulkanInstance::get().getDevice(), pool, 0);
}

VkDescriptorSet DescriptorAllocator::allocate(VkDescriptorSetLayout layout) {
	VkDescriptorSetAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet set;
    vkAllocateDescriptorSets(VulkanInstance::get().getDevice(), &allocInfo, &set);

	return set;
}
