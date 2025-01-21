#include "vulkanPlatformBridge.h"

#include <QVulkanInstance>

#include "vulkanManager.h"

// this file exists because apple is stupid

std::vector<std::string> getRequiredInstanceExtensions() {
#ifdef __APPLE__
	
#else
	// goofy ahh hack to get required extension list
	QVulkanInstance tempInstance;
	tempInstance.create();
	QByteArrayList qExtensions = tempInstance.extensions();
	std::vector<std::string> extensions(qExtensions.begin(), qExtensions.end());
	// std::cout << extensions[0] << std::endl;
	// std::cout << extensions[1] << std::endl;
	// std::cout << extensions[2] << std::endl;
	// std::cout << extensions[3] << std::endl;
	tempInstance.destroy();
	
	return extensions;
	
#endif
}

VulkanSurface::VulkanSurface(QWindow* window) {
#ifdef __APPLE__
	
#else
	qVulkanInstance.setVkInstance(Vulkan::Instance());
	qVulkanInstance.create();
	window->setVulkanInstance(&qVulkanInstance);
	surface = QVulkanInstance::surfaceForWindow(window);
#endif
}

VulkanSurface::~VulkanSurface() {
	
}
