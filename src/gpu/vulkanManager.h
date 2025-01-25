#ifndef vulkanManager_h
#define vulkanManager_h

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "gpu/vulkanDevice.h"

class Vulkan {
public:
	Vulkan() = default;
	Vulkan(const Vulkan& o) = delete;
	void operator=(const Vulkan& o) = delete;

	// singleton access
	inline static Vulkan& getSingleton() { static Vulkan vulkan; return vulkan; }
	inline static const VkInstance& getInstance() { return getSingleton().instance; }
	inline static const VkPhysicalDevice& getPhysicalDevice() { return getSingleton().physicalDevice; }
	inline static const VkDevice& getDevice() { return getSingleton().device; }
	inline static const VmaAllocator& getAllocator() { return getSingleton().allocator; }
	inline static const QueueFamilies& getQueueFamilies() { return getSingleton().queueFamilies; }

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
	void createAllocator();
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR idealSurface);
	void createLogicalDevice();
	
private:
	// important shit
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VmaAllocator allocator;
	QueueFamilies queueFamilies;

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
//       - [x] Memory allocation
// - [x] Drawing with vertex buffers
// - [ ] Prototype block renderer with
//       - [ ] Push constant view matrix
//       - [ ] Blocks and textures
// - [ ] Butter buffer management. Staging buffer?
// - [ ] New object management scheme
//       -  Possibly RAII?
//       -  Possibly deletion queue
//       -  Builder pattern (for pipeline at least)
// - [ ] Don't draw directly to swapchain
// - [ ] IMGUI for debug
// - [ ] Switch from singleton to top down design

#endif
