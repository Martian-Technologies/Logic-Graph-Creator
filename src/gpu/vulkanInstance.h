#ifndef vulkanInstance_h
#define vulkanInstance_h

#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

struct QueueInfo {
	VkQueue queue;
	uint32_t index;
};

class VulkanInstance {
private:
	static VulkanInstance* singleton;
public:
	static inline VulkanInstance& get() { return *singleton; }
public:
	VulkanInstance();
	~VulkanInstance();

	void ensureDeviceCreation(VkSurfaceKHR surfaceForPresenting);
	void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

public:
	// getters
	inline VkInstance& getInstance() { return instance.instance; }
	inline VmaAllocator& getAllocator() { return allocator.value(); }
	inline vkb::Device& getVkbDevice() { return device.value(); }
	inline VkDevice& getDevice() { return device.value().device; }
	inline QueueInfo& getGraphicsQueue() { return graphicsQueue.value(); }
	inline std::mutex& getGraphicsSubmitMux() { return graphicsSubmitMux; }
	inline QueueInfo& getPresentQueue() { return presentQueue.value(); }
	inline std::mutex& getPresentSubmitMux() { return presentSubmitMux; }

private:
	void createAllocator();
	void initializeImmediateSubmission();
	
private:
	vkb::Instance instance;
	std::optional<VkPhysicalDevice> physicalDevice;
	std::optional<vkb::Device> device;
	std::optional<VmaAllocator> allocator;

	// Queues
	std::optional<QueueInfo> graphicsQueue;
	std::mutex graphicsSubmitMux;
	std::optional<QueueInfo> presentQueue;
	std::mutex presentSubmitMux;

	// Immediate submission
    VkFence immediateFence;
    VkCommandBuffer immediateCommandBuffer;
    VkCommandPool immediateCommandPool;
};

// Vulkan Renderer Design Notes
// - most of the renderer isn't designed to be modular or particular extensible
// right now, the goal is just to logically separate each "component" into its
// own file
// - from what we do have right now, VulkanManager is a singleton that has
// global vulkan objects, and will be in charge of distributing certain
// resources (like queues). Each vulkan renderer will be in charge of managing
// its own stuff.

// VULKAN TODO -
// - [ ] Vulkan chunker with wires - Chunk system should be abstracted somewaht (just also supports wires, move a few functions out)
// - [ ] Better naming convention for classes
// - [ ] Block textures
// - [ ] Basic effects

// - [ ] Chunks are allocated on separate threads, buffer pooling that replaces GBJ
// - [ ] Vertex generation
// - [ ] More gradual chunking sync to allow more parallelism
// - [ ] Staging Buffers

//
// VULKAN IMPROVEMENTS -
// - [x] VkBootstrap
// - [ ] Switch from singleton to top down design
// - [ ] Fix validation layers on mac, and weird resize messages
// - [ ] Standardization of subrenderer params and input, better way for subrenderer to communicate and put data on the "frame", growable descriptor pool
// - [ ] Check macro
// - [ ] Use dynamic rendering and other QOL extensions to simplify code
// - [ ] Don't draw directly to swapchain

#endif
