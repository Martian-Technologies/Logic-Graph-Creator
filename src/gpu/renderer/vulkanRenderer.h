#ifndef vulkanRenderer_h
#define vulkanRenderer_h

#include "gui/circuitView/renderer/renderer.h"

#include "vulkanCircuitBufferRing.h"
#include "vulkanSwapchain.h"
#include "vulkanPipeline.h"
#include "vulkanFrame.h"

// STATES of a vulkan renderer:
// - constructed
// - initialized
// - running / not running

constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanRenderer : public Renderer {
	// VULKAN -----------------------------------------------------------------------------
	// ------------------------------------------------------------------------------------
public:
	void initialize(VkSurfaceKHR surface, int w, int h);
	void destroy();

	// screen size updates should be sent regardless of state
	void resize(int w, int h);
	
	void run();
	void stop();

private:
	void recordCommandBuffer(FrameData& frame, uint32_t imageIndex);
	
private:
	// state
	std::atomic<bool> initialized = false;
	std::atomic<bool> running = false;
	int windowWidth, windowHeight;
	std::mutex cpuRenderingMutex;
	glm::mat4 orthoMat = glm::mat4(1.0f);

	// screen and frame
	VkSurfaceKHR surface;
	SwapchainData swapchain;
	FrameData frames[FRAME_OVERLAP];

	// pipeline
	VulkanCircuitBufferRing circuitBufferRing;
	PipelineData pipeline;
	VkShaderModule vertShader;
	VkShaderModule fragShader;

	// render loop
	std::thread renderThread;
	void renderLoop();

	// frame counting
	int frameNumber = 0;
	inline FrameData& getCurrentFrame() { return frames[frameNumber % FRAME_OVERLAP]; };
	
	// INTERFACE --------------------------------------------------------------------------
	// ------------------------------------------------------------------------------------
public:
	// updating
	void setCircuit(Circuit* circuit) override;
	void setEvaluator(Evaluator* evaluator) override;

	void updateView(ViewManager* viewManager) override;
	virtual void updateCircuit(DifferenceSharedPtr diff) override;

	inline float getLastFrameTimeMs() const override { return lastFrameTime; }

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
