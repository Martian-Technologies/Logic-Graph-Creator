#ifndef vulkanInstance_h
#define vulkanInstance_h

#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

struct QueueInfo {
	VkQueue queue;
	uint32_t index;
};

class VulkanInstance {
private:
	static VulkanInstance* singleton;
public:
	static inline VulkanInstance& get() { return *singleton; }
public:
	VulkanInstance();
	~VulkanInstance();

	void ensureDeviceCreation(VkSurfaceKHR surfaceForPresenting);

public:
	inline VkInstance& getInstance() { return instance.instance; }
	inline vkb::Device& getVkbDevice() { return device.value(); }
	inline VkDevice& getDevice() { return device.value().device; }
	inline QueueInfo& getGraphicsQueue() { return graphicsQueue.value(); }
	inline QueueInfo& getPresentQueue() { return presentQueue.value(); }

private:
	void createAllocator();
	
private:
	vkb::Instance instance;
	std::optional<VkPhysicalDevice> physicalDevice;
	std::optional<vkb::Device> device;
	std::optional<VmaAllocator> allocator;

	// Queues
	std::optional<QueueInfo> graphicsQueue;
	std::optional<QueueInfo> presentQueue;
};

#endif
