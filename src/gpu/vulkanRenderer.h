#ifndef vulkanRenderer_h
#define vulkanRenderer_h

#include "gui/circuitView/renderer/renderer.h"

#include "gpu/vulkanSwapchain.h"
#include "gpu/vulkanManager.h"
#include "gpu/vulkanFrame.h"

constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanRenderer : public Renderer {
	// VULKAN -----------------------------------------------------------------------------
	// ------------------------------------------------------------------------------------
public:
	void initialize(VulkanGraphicsView view, VkSurfaceKHR surface, int w, int h);
	void destroy();
	void resize(int w, int h);
	void run();

private:
	inline FrameData& get_current_frame() { return frames[frameNumber % FRAME_OVERLAP]; };
	
private:
	VulkanGraphicsView view;
	VkSurfaceKHR surface;
	SwapchainData swapchain;
	FrameData frames[FRAME_OVERLAP];

	int frameNumber = 0;
	int windowWidth, windowHeight;

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
