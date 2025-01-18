#ifndef vulkanManager_h
#define vulkanManager_h

#include <vulkan/vulkan.h>

#include "gpu/vulkanDevice.h"

struct VulkanGraphicsView {
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	QueueFamilies queueFamilies;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
};

// TODO - global vulkan context?
class VulkanManager {
public:
	// flow
	void createInstance(const std::vector<const char*>& requiredExtensions);
	void setUpDevice(VkSurfaceKHR surface);
	void destroy();

	// util
	inline VkInstance getInstance() const { return instance; }
	VulkanGraphicsView createGraphicsView();

private:
	// helper functions
	void fail(const std::string& reason);

	bool checkValidationLayerSupport();
	void pickPhysicalDevice(VkSurfaceKHR idealSurface);
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR idealSurface);
	void createLogicalDevice(VkSurfaceKHR surface);
	
private:
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;

	QueueFamilies queueFamilies;
	std::vector<VkQueue> graphicsQueues;
	std::vector<VkQueue> presentQueues;
	// TODO - temporary round robin queue distributions
	int graphicsRoundRobin;
	int presentRoundRobin;
};

// VULKAN TODO -
// - [ ] Global vulkan state that handles instance/device
//       - [ ] Queue management
//       - [ ] Memory allocation
//       - [ ] Deletion

#endif
