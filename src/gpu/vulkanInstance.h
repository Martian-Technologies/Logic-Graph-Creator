#ifndef vulkanInstance_h
#define vulkanInstance_h

#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

class VulkanInstance {
private:
	static VulkanInstance* singleton;
public:
	static inline VulkanInstance& get() { return *singleton; }
public:
	VulkanInstance();
	~VulkanInstance();

	void ensureDeviceCreation(VkSurfaceKHR surfaceForPresenting);
	
private:
	vkb::Instance instance;
	std::optional<vkb::Device> device;
};

#endif
