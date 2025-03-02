#ifndef vulkanRenderer_h
#define vulkanRenderer_h

#include "gui/circuitView/renderer/renderer.h"

#include "vulkanBlockRenderer.h"
#include "vulkanSwapchain.h"
#include "vulkanFrame.h"

#include <glm/mat4x4.hpp>

// STATES of a vulkan renderer:
// - constructed
// - initialized
// - running / not running

constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanRenderer : public Renderer {
public:
	// setup and management
	void initialize(VkSurfaceKHR surface, int w, int h);
	void destroy();

	void run();
	void stop();

	// cpu synchronized with render loop
	void resize(int w, int h);
	void updateView(ViewManager* viewManager) override;

	// synchronized separately (independent of render loop) (will not block thread for render loop)
	void setCircuit(Circuit* circuit) override;
	void updateCircuit(DifferenceSharedPtr diff) override;
	void setEvaluator(Evaluator* evaluator) override;

	inline float getLastFrameTimeMs() const override { return lastFrameTime; }

private:
	void recordCommandBuffer(FrameData& frame, uint32_t imageIndex);
	void createRenderPass(SwapchainData& swapchain);
	
private:
	// state
	std::atomic<bool> initialized = false;
	std::atomic<bool> running = false;
	int windowWidth, windowHeight;
	glm::mat4 orthoMat = glm::mat4(1.0f);
	std::mutex cpuRenderingMutex;

	// vulkan objects
	VkSurfaceKHR surface;
	VkRenderPass renderPass;
	SwapchainData swapchain;
	FrameData frames[FRAME_OVERLAP];

	// sub renderers
	VulkanBlockRenderer blockRenderer;

	// render loop
	std::thread renderThread;
	void renderLoop();

	// frame counting
	int frameNumber = 0;
	inline FrameData& getCurrentFrame() { return frames[frameNumber % FRAME_OVERLAP]; };

private:
	// elements
	ElementID addSelectionElement(const SelectionObjectElement& selection) override;
	ElementID addSelectionElement(const SelectionElement& selection) override;
	void removeSelectionElement(ElementID selection) override;

	ElementID addBlockPreview(const BlockPreview& blockPreview) override;
	void removeBlockPreview(ElementID blockPreview) override;

	ElementID addConnectionPreview(const ConnectionPreview& connectionPreview) override;
	void removeConnectionPreview(ElementID connectionPreview) override;

	ElementID addHalfConnectionPreview(const HalfConnectionPreview& halfConnectionPreview) override;
	void removeHalfConnectionPreview(ElementID halfConnectionPreview) override;

	void spawnConfetti(FPosition start) override;

private:
	float lastFrameTime = 0.0f;

};

#endif
