#ifndef vulkanDevice_h
#define vulkanDevice_h

#include <vulkan/vulkan.h>

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamily {
	uint32_t index;
	uint32_t queueCount;

	inline bool operator<(const QueueFamily& other) const { return index < other.index; }
	inline bool operator==(const QueueFamily& other) const { return index == other.index;	}
};

struct QueueFamilies {
	std::optional<QueueFamily> graphicsFamily;
	std::optional<QueueFamily> presentFamily;

	inline bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

QueueFamilies findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR idealSurface);
bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& requiredExtensions);
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR idealSurface);

#endif
