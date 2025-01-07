#ifndef vulkanManager_h
#define vulkanManager_h

#include <vulkan/vulkan.h>

struct VulkanView {
    VkDevice device;
};

class VulkanManager {
public:
    void createInstance(const std::vector<const char*>& requiredExtensions);
    void createDevice(VkSurfaceKHR surface);

    void destroy();

    // util
    inline VkInstance getInstance() const { return instance; }
    inline VulkanView createVulkanView() const { return { device }; }

private:
    VkInstance instance;
    VkDevice device;
};

#endif
