#ifndef windowRenderingManager_h
#define windowRenderingManager_h

#include <RmlUi/Core/RenderInterface.h>
#include <thread>

#include "gpu/renderer/windowRenderer.h"
#include "gui/rml/rmlRenderInterface.h"
#include "gui/sdl/sdlWindow.h"
#include "vulkanFrame.h"
#include "vulkanSwapchain.h"

constexpr unsigned int FRAMES_IN_FLIGHT = 1;

class WindowRenderingManager {
public:
	WindowRenderingManager(SdlWindow* sdlWindow);
	~WindowRenderingManager();

	// no copy
	WindowRenderingManager(const WindowRenderingManager&) = delete;
	WindowRenderingManager& operator=(const WindowRenderingManager&) = delete;

public:
	void resize(std::pair<uint32_t, uint32_t> windowSize);
	
	void prepareForRml(RmlRenderInterface& renderInterface);
	void endRml();

	void registerViewportRenderInterface(ViewportRenderInterface* renderInterface);
	void deregisterViewportRenderInterface(ViewportRenderInterface* renderInterface);

private:
	void recreateSwapchain();
	
private:
	SdlWindow* sdlWindow;
	VkDevice device;
	std::atomic<bool> running = false;

	// main vulkan
	VkSurfaceKHR surface;
	std::unique_ptr<Swapchain> swapchain = nullptr;
	std::atomic<bool> swapchainRecreationNeeded = false;
	std::unique_ptr<WindowRenderer> subrenderer = nullptr;

	// size
	std::pair<uint32_t, uint32_t> windowSize;
	std::mutex windowSizeMux;

	// frames
	std::vector<VulkanFrameData> frames; // TODO - (this should be a std array once we have proper RAII)
	int frameNumber = 0;
	inline VulkanFrameData& getCurrentFrame(int offset = 0) { return frames[(frameNumber + offset) % FRAMES_IN_FLIGHT]; };

	// render loop
	std::thread renderThread;
	void renderLoop();
};

#endif
