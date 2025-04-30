#ifndef vulkanFrame_h
#define vulkanFrame_h

#include <vulkan/vulkan.h>
#include <chrono>
#include <glm/ext/matrix_float4x4.hpp>

#include "util/lifetimeExtender.h"

class VulkanFrameData {
public:
	VulkanFrameData();
	~VulkanFrameData();

	void start();
	float waitAndComplete();

	inline VkCommandPool& getCommandPool() { return commandPool; };
	inline VkCommandBuffer& getMainCommandBuffer() { return mainCommandBuffer; };
	inline VkSemaphore& getSwapchainSemaphore() { return swapchainSemaphore; };
	inline VkSemaphore& getRenderSemaphore() { return renderSemaphore; }
	inline VkFence& getRenderFence() { return renderFence; };
	inline LifetimeExtender& getFrameLifetime() { return frameLifetime; };
	
private:
	// mostly static core data
	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;
	VkSemaphore swapchainSemaphore, renderSemaphore;
	VkFence renderFence;

	// lifetime extender to be used by subrenderers which want to put objects "on" the frame
	LifetimeExtender frameLifetime;

	std::chrono::time_point<std::chrono::system_clock> lastStartTime;
	
};

#endif
