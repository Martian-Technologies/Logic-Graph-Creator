#ifndef vulkanPlatformBridge_h
#define vulkanPlatformBridge_h

#include <QWindow>
#include <vulkan/vulkan.h>

std::vector<std::string> getRequiredInstanceExtensions();
VkSurfaceKHR createSurface(QWindow *window);
void destroySurface(VkSurfaceKHR surface);

#endif
