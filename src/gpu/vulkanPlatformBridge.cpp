#include "vulkanPlatformBridge.h"

#include "vulkanManager.h"

#ifdef __APPLE__
#include <vulkan/vulkan_macos.h>
#endif

// this file exists because apple is stupid

std::vector<std::string> getRequiredInstanceExtensions() {
#ifdef __APPLE__
	// we don't have a library for this on apple, so this will have to work. May not age very well
	std::vector<std::string> extensions = { "VK_KHR_surface", "VK_KHR_portability_enumeration", "VK_MVK_macos_surface"};
	// std::vector<std::string> extensions = { "VK_KHR_surface", "VK_KHR_portability_enumeration", "VK_EXT_DEBUG_UTILS", "VK_EXT_metal_surface"};
	return extensions;
	
#else
	// goofy ahh hack to get required extension list
	QVulkanInstance tempInstance;
	tempInstance.create();
	QByteArrayList qExtensions = tempInstance.extensions();
	std::vector<std::string> extensions(qExtensions.begin(), qExtensions.end());
	tempInstance.destroy();
	
	return extensions;
	
#endif
}

VulkanSurface::VulkanSurface(QWindow* window) {
#ifdef __APPLE__
	VkMacOSSurfaceCreateInfoMVK surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
	auto winID = window->winId();
	// we're going to hope this works
	surfaceInfo.pView = reinterpret_cast<const void*>(winID);

	PFN_vkCreateMacOSSurfaceMVK vkCreateMacOSSurfaceMVK =
		(PFN_vkCreateMacOSSurfaceMVK)vkGetInstanceProcAddr(Vulkan::Instance(), "vkCreateMacOSSurfaceMVK");

	vkCreateMacOSSurfaceMVK(Vulkan::Instance(), &surfaceInfo, nullptr, &surface);
#else
	qVulkanInstance.setVkInstance(Vulkan::Instance());
	qVulkanInstance.create();
	window->setVulkanInstance(&qVulkanInstance);
	surface = QVulkanInstance::surfaceForWindow(window);
#endif
}

VulkanSurface::~VulkanSurface() {
#ifdef __APPLE__
	vkDestroySurfaceKHR(Vulkan::Instance(), surface, nullptr);
#else
	// QT does it
#endif
}
