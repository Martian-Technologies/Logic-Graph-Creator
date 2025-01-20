#ifndef vulkanManager_h
#define vulkanManager_h

#include <vulkan/vulkan.h>

#include "gpu/vulkanDevice.h"

class Vulkan {
public:
	Vulkan() = default;
	Vulkan(const Vulkan& o) = delete;
	void operator=(const Vulkan& o) = delete;

	// singleton access
	inline static Vulkan& Singleton() { static Vulkan vulkan; return vulkan; }
	inline static const VkInstance& Instance() { return Singleton().instance; }
	inline static const VkPhysicalDevice& PhysicalDevice() { return Singleton().physicalDevice; }
	inline static const VkDevice& Device() { return Singleton().device; }
	inline static const QueueFamilies& QueueFamilies() { return Singleton().queueFamilies; }

public:
	// management functions
	void createInstance(const std::vector<const char*>& requiredExtensions);
	void setupDevice(VkSurfaceKHR surface);
	void destroy();

	VkQueue& requestGraphicsQueue(bool important = false);
	VkQueue& requestPresentQueue(bool important = false);

private:
	// helper functions
	bool checkValidationLayerSupport();
	void pickPhysicalDevice(VkSurfaceKHR idealSurface);
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR idealSurface);
	void createLogicalDevice();
	
private:
	// important shit
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	struct QueueFamilies queueFamilies;

	// stupid shit
	std::vector<VkQueue> graphicsQueues;
	std::vector<VkQueue> presentQueues;
	// TODO - temporary round robin queue distributions
	int graphicsRoundRobin = 0;
	int presentRoundRobin = 0;
};

// VULKAN TODO -
// - [ ] Create the surface ourself
// - [ ] Global vulkan state that handles instance/device
//       - [ ] Queue management
//       - [ ] Memory allocation
//       - [ ] Deletion

#endif
