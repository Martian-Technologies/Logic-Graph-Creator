#ifndef vulkanManager_h
#define vulkanManager_h

#include "gpu/vulkanDevice.h"
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
	void pickPhysicalDevice(VkSurfaceKHR idealSurface);
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR idealSurface);
	
private:
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;

	QueueFamilyIndices queueIndices;
};

#endif
