#ifndef vulkanPlatformBridge_h
#define vulkanPlatformBridge_h

#include <vulkan/vulkan.h>

#include <QWindow>
#include <QVulkanInstance>

std::vector<std::string> getRequiredInstanceExtensions();

class VulkanSurface {
public:
	// Requires a QWindow that is shown and has its surface type set to vulkan
	VulkanSurface(QWindow* window);
	~VulkanSurface();

	inline const VkSurfaceKHR& getVkSurfaceKHR() const { return surface; }

	// we do not fuck around with this object
	VulkanSurface(const VulkanSurface&) = delete;
	VulkanSurface(VulkanSurface&&) = delete;
	VulkanSurface& operator=(const VulkanSurface&) = delete;
	VulkanSurface& operator=(VulkanSurface&&) = delete;

private:
	VkSurfaceKHR surface;
#ifndef __APPLE__
	// if not on apple, we need a qVulkanInstance to manage surface
	QVulkanInstance qVulkanInstance;
#endif // __APPLE__
};

#endif
