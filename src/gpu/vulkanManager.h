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
	void createInstance();
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

// Vulkan Renderer Design Notes
// - most of the renderer isn't designed to be modular or particular extensible
// right now, the goal is just to logically separate each "component" into its
// own file
// - from what we do have right now, VulkanManager is a singleton that has
// global vulkan objects, and will be in charge of distributing certain
// resources (like queues). Each vulkan renderer will be in charge of managing
// its own stuff.


// VULKAN TODO -
// - [x] New surface creation
// - [x] Global vulkan state that handles instance/device
//       - [x] Queue requests
//       - [ ] Memory allocation
// - [ ] Drawing with vertex buffers
// - [ ] Prototype block renderer
// - [ ] New object management scheme
//       -  Possibly RAII?
//       -  Possibly deletion queue
//       -  Builder pattern 
// - [ ] Don't draw directly to swapchain
// - [ ] IMGUI for debug

#endif
