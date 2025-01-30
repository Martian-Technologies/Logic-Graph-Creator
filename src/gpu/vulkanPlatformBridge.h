#ifndef vulkanPlatformBridge_h
#define vulkanPlatformBridge_h

#include <vulkan/vulkan.h>

#include <QWindow>
#include <QVulkanInstance>

std::vector<std::string> getRequiredInstanceExtensions();

#endif
