#ifndef vulkanManager_h
#define vulkanManager_h

#include <vulkan/vulkan.h>

struct VulkanGraphicsView {
	VkDevice device;
};

class VulkanManager {
public:
	// flow
	void createInstance(const std::vector<const char*>& requiredExtensions, bool enableValidationLayers);
	void createDevice(VkSurfaceKHR surface);
	void destroy();

	// util
	inline VkInstance getInstance() const { return instance; }
	inline VulkanGraphicsView createVulkanGraphicsView() const { return { device }; }

private:
	void fail(const std::string& reason);
	// helper functions
	bool checkValidationLayerSupport();
	
private:
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
};

#endif
