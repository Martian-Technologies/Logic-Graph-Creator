#include "vulkanQtWindow.h"

#include "vulkanManager.h"

#ifdef __APPLE__
#include <vulkan/vulkan_macos.h>
#endif

VulkanQtWindow::VulkanQtWindow() {
	setSurfaceType(QSurface::VulkanSurface);
}

VkSurfaceKHR VulkanQtWindow::createSurface() {
#ifdef __APPLE__
	VkMacOSSurfaceCreateInfoMVK surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
	auto winID = winId();
	// we're going to hope this works
	surfaceInfo.pView = reinterpret_cast<const void*>(winID);

	PFN_vkCreateMacOSSurfaceMVK vkCreateMacOSSurfaceMVK =
		(PFN_vkCreateMacOSSurfaceMVK)vkGetInstanceProcAddr(Vulkan::getInstance(), "vkCreateMacOSSurfaceMVK");

	vkCreateMacOSSurfaceMVK(Vulkan::getInstance(), &surfaceInfo, nullptr, &surface);
#else
	qVulkanInstance.setVkInstance(Vulkan::getInstance());
	qVulkanInstance.create();
	setVulkanInstance(&qVulkanInstance);
	surface = QVulkanInstance::surfaceForWindow(this);
#endif

	return surface;
}

void VulkanQtWindow::destroySurface() {
#ifdef __APPLE__
	vkDestroySurfaceKHR(Vulkan::getInstance(), surface, nullptr);
#else
	// QT does it
#endif
}
