#ifndef vulkanDevicePicker_h
#define vulkanDevicePicker_h

#include <vulkan/vulkan.h>

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

class VulkanDevicePicker {
public:
	VulkanDevicePicker(VkInstance instance, VkSurfaceKHR surface);

	inline void requireExtensions(const std::vector<std::string>& requiredDeviceExtensions) { this->requiredDeviceExtensions = requiredDeviceExtensions; }
	
	std::optional<VkPhysicalDevice> pick();
	inline const std::string& getError() const { return error; };
	
private:
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

private:
	VkInstance instance;
	VkSurfaceKHR idealSurface;
	
	std::optional<std::vector<std::string>> requiredDeviceExtensions = std::nullopt;
	
	std::string error;
};

#endif
