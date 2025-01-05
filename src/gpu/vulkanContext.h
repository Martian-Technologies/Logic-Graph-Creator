#ifndef vulkanContext_h
#define vulkanContext_h

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

// probably the only vulkan class that will use RAII

class VulkanContext {
public:
    VulkanContext();
    ~VulkanContext();

    inline VkInstance getInstance() const { return instance; }
    inline VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    inline VkDevice getDevice() const { return device; }

private:
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
};

#endif
