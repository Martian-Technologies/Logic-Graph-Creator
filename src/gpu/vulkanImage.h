#ifndef vulkanImage_h
#define vulkanImage_h

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

struct AllocatedImage {
    VkImage image;
    VkImageView imageView;
    VmaAllocation allocation;
    VkExtent3D imageExtent;
    VkFormat imageFormat;
	VkImageAspectFlags aspect;
	uint32_t mipLevels;
};

AllocatedImage createImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
AllocatedImage createImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
void destroyImage(AllocatedImage& image);

bool transitionImageLayout(VkCommandBuffer cmd, AllocatedImage& image, VkImageLayout oldLayout, VkImageLayout newLayout);

#endif
