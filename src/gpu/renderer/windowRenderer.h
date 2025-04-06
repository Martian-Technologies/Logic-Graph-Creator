#ifndef windowRenderer_h
#define windowRenderer_h

#include <RmlUi/Core/RenderInterface.h>
#include <thread>
#include <glm/mat4x4.hpp>

#include "gui/rml/rmlRenderInterface.h"
#include "gui/sdl/sdlWindow.h"
#include "rmlRenderer.h"
#include "vulkanFrame.h"
#include "vulkanSwapchain.h"

constexpr unsigned int FRAMES_IN_FLIGHT = 1;

class WindowRenderer {
public:
	WindowRenderer(SdlWindow* sdlWindow);
	~WindowRenderer();

	// no copy
	WindowRenderer(const WindowRenderer&) = delete;
	WindowRenderer& operator=(const WindowRenderer&) = delete;

public:
	void resize(std::pair<uint32_t, uint32_t> windowSize);
	
	void prepareForRml(RmlRenderInterface& renderInterface);
	void endRml();

private:
	void createRenderPass();
	void recreateSwapchain();
	void recordCommandBuffer(VulkanFrameData& frame, uint32_t imageIndex);
	
private:
	SdlWindow* sdlWindow;
	VkDevice device;
	std::atomic<bool> running = false;

	// main vulkan
	VkSurfaceKHR surface;
	VkRenderPass renderPass;
	std::unique_ptr<Swapchain> swapchain = nullptr;
	std::atomic<bool> swapchainRecreationNeeded = false;

	// size
	std::pair<uint32_t, uint32_t> windowSize;
	glm::mat4 pixelViewMat;
	std::mutex windowSizeMux;
	
	// frames
	std::vector<VulkanFrameData> frames; // TODO - (this should be a std array once we have proper RAII)
	int frameNumber = 0;
	inline VulkanFrameData& getCurrentFrame(int offset = 0) { return frames[(frameNumber + offset) % FRAMES_IN_FLIGHT]; };

	// rml
	std::unique_ptr<RmlRenderer> rmlRenderer = nullptr;

	// render loop
	std::thread renderThread;
	void renderLoop();
};

#endif
