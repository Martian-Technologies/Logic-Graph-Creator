#ifndef vulkanInstance_h
#define vulkanInstance_h

#include <volk.h>
#include <VkBootstrap.h>

#include "vulkanDevice.h"

class VulkanInstance {
public:
	VulkanInstance();
	~VulkanInstance();

	VulkanDevice* createOrGetDevice(VkSurfaceKHR surfaceForPresenting);

	inline vkb::Instance getVkbInstance() { return instance; };

private:
	vkb::Instance instance;
	std::optional<VulkanDevice> device;
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
// - [x] Vulkan chunker with wires - Chunk system should be abstracted somewaht (just also supports wires, move a few functions out)
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
// - [x] Switch from singleton to top down design
// - [x] Volk dynamic loader
// - [ ] Pooled async resource uploading
// - [ ] Standardization of subrenderer params and input, better way for subrenderer to communicate and put data on the "frame", growable descriptor pool
// - [ ] Fix validation layers on mac, and weird resize messages on x11
// - [ ] Check macro
// - [ ] Use dynamic rendering, push descriptors and other QOL extensions to simplify code
// - [ ] Don't draw directly to swapchain

#endif
