#ifndef vulkanDescriptor_h
#define vulkanDescriptor_h

#include <vulkan/vulkan.h>

class DescriptorLayoutBuilder {
public:
	void addBinding(uint32_t bindingIndex, VkDescriptorType type);
	VkDescriptorSetLayout build(VkShaderStageFlags shaderStages, VkDescriptorSetLayoutCreateFlags flags = 0);
	
private:
	std::vector<VkDescriptorSetLayoutBinding> bindings;
};

class DescriptorWriter {
public:
	void writeImage(int bindingIndex, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type);
    void writeBuffer(int bindingIndex, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type); 

    void updateSet(VkDevice device, VkDescriptorSet set);
	
private:
	std::deque<VkDescriptorImageInfo> imageInfos;
    std::deque<VkDescriptorBufferInfo> bufferInfos;
    std::vector<VkWriteDescriptorSet> writes;
};

class DescriptorAllocator {
public:
	struct PoolSizeRatio {
		VkDescriptorType type;
		float ratio;
    };
public:
	DescriptorAllocator(uint32_t maxSets, const std::vector<PoolSizeRatio>& poolRatios);
	~DescriptorAllocator();

	void clearDescriptors();
	VkDescriptorSet allocate(VkDescriptorSetLayout layout);

private:
	VkDescriptorPool pool;
};

#endif
