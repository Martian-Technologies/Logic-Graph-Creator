#ifndef windowRenderer_h
#define windowRenderer_h

#include <RmlUi/Core/RenderInterface.h>
#include <thread>

#include "gui/rml/rmlRenderInterface.h"
#include "gui/sdl/sdlWindow.h"

#include "gpu/renderer/frameManager.h"
#include "gpu/renderer/viewport/viewportRenderer.h"
#include "gpu/renderer/viewportRenderInterface.h"
#include "gpu/abstractions/vulkanSwapchain.h"

class WindowRenderer {
public:
	WindowRenderer(SdlWindow* sdlWindow, VulkanInstance* instance);
	~WindowRenderer();

	// no copy
	WindowRenderer(const WindowRenderer&) = delete;
	WindowRenderer& operator=(const WindowRenderer&) = delete;

public:
	void resize(std::pair<uint32_t, uint32_t> windowSize);

	void activateRml(RmlRenderInterface& renderInterface);
	void prepareForRml(RmlRenderInterface& renderInterface);
	void endRml();

	void registerViewportRenderInterface(ViewportRenderInterface* renderInterface);
	void deregisterViewportRenderInterface(ViewportRenderInterface* renderInterface);

	inline VulkanDevice* getDevice() { return device; }

private:
	void renderToCommandBuffer(Frame& frame, uint32_t imageIndex);
	void createRenderPass();
	void recreateSwapchain();
	
private:
	// screen
	Swapchain swapchain;
	std::atomic<bool> swapchainRecreationNeeded = false;
	std::pair<uint32_t, uint32_t> windowSize;
	std::mutex windowSizeMux;
	
	// main vulkan
	VkSurfaceKHR surface;
	VkRenderPass renderPass;

	// subrenderers
	RmlRenderer rmlRenderer;
	ViewportRenderer viewportRenderer;

	// render loop
	FrameManager frames;
	std::thread renderThread;
	std::atomic<bool> running = false;
	void renderLoop();

	// connected viewport render interfaces
	std::set<ViewportRenderInterface*> viewportRenderInterfaces;
	std::mutex viewportRenderersMux;

	// handles
	SdlWindow* sdlWindow;
	VulkanDevice* device;
};

#endif
