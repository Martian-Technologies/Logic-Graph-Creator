#ifndef vulkanUtil_h
#define vulkanUtil_h

#include <vulkan/vulkan.h>

VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags /*= 0*/);
void transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
VkImageSubresourceRange imageSubresourceRange(VkImageAspectFlags aspectMask);

#endif
