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
	WindowRenderer(SdlWindow* sdlWindow);
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

private:
	void renderToCommandBuffer(Frame& frame, uint32_t imageIndex);
	void createRenderPass();
	void recreateSwapchain();
	
private:
	SdlWindow* sdlWindow;
	VkDevice device;
	std::atomic<bool> running = false;

	// size
	std::pair<uint32_t, uint32_t> windowSize;
	std::mutex windowSizeMux;
	
	// main vulkan
	VkSurfaceKHR surface;
	std::unique_ptr<Swapchain> swapchain = nullptr;
	std::atomic<bool> swapchainRecreationNeeded = false;
	VkRenderPass renderPass;

	// subrenderers
	std::unique_ptr<RmlRenderer> rmlRenderer = nullptr;
	std::unique_ptr<ViewportRenderer> viewportRenderer = nullptr;

	// connected viewport render interfaces
	std::set<ViewportRenderInterface*> viewportRenderInterfaces;
	std::mutex viewportRenderersMux;

	std::unique_ptr<FrameManager> frames;

	// render loop
	std::thread renderThread;
	void renderLoop();
};

#endif
