#ifndef vulkanQtWindow_h
#define vulkanQtWindow_h

#include <QWindow>
#include <QVulkanInstance>
#include <vulkan/vulkan.h>

class VulkanQtWindow : public QWindow {
	Q_OBJECT
public:
	VulkanQtWindow();

	// we do not fuck around with this object
	VulkanQtWindow(const VulkanQtWindow&) = delete;
	VulkanQtWindow(VulkanQtWindow&&) = delete;
	VulkanQtWindow& operator=(const VulkanQtWindow&) = delete;
	VulkanQtWindow& operator=(VulkanQtWindow&&) = delete;

public:
	VkSurfaceKHR createSurface();
	void destroySurface();

private:
	VkSurfaceKHR surface;
#ifndef __APPLE__
	// if not on apple, we need a qVulkanInstance to create the surface
	QVulkanInstance qVulkanInstance;
#endif
};

#endif
