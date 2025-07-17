#ifndef vulkanDevice_h
#define vulkanDevice_h

#include <volk.h>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

class VulkanInstance;
class BlockTextureManager;

struct QueueInfo {
	VkQueue queue;
	uint32_t index;
};

class VulkanDevice {
public:
	VulkanDevice(VulkanInstance* instance, VkSurfaceKHR surfaceForPresenting);
	~VulkanDevice();

	// queue submission functions
	void waitIdle();
	VkResult submitGraphicsQueue(VkSubmitInfo* submitInfo, VkFence fence);
	VkResult submitPresent(VkPresentInfoKHR* presentInfo);
	void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

	inline uint32_t getGraphicsQueueIndex() const { return graphicsQueue.index; }
	inline uint32_t getPresentQueueIndex() const { return presentQueue.index; }

	inline vkb::Device& getDevice() { return device; }
	inline VmaAllocator getAllocator() const { return vmaAllocator; }

	inline BlockTextureManager* getBlockTextureManager() const { return blockTextureManager.get(); }
	
private:
	void createAllocator();
	void initializeImmediateSubmission();
	
private:
	// Device details
	VkPhysicalDevice physicalDevice;
	vkb::Device device;
	VmaAllocator vmaAllocator;
	VulkanInstance* instance;

	// Queues
	QueueInfo graphicsQueue;
	QueueInfo presentQueue;
	std::mutex queueMux;

	// Immediate submission system
	VkFence immediateFence;
    VkCommandBuffer immediateCommandBuffer;
    VkCommandPool immediateCommandPool;
	std::mutex immediateSubmitMux;

	// Texture
	std::unique_ptr<BlockTextureManager> blockTextureManager;
};

#endif
