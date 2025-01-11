#ifndef vulkanRenderer_h
#define vulkanRenderer_h

#include "gui/circuitView/renderer/renderer.h"

#include "gpu/vulkanManager.h"

class VulkanRenderer : public Renderer {
	// VULKAN -----------------------------------------------------------------------------
	// ------------------------------------------------------------------------------------
public:
	void initialize(VulkanGraphicsView view, VkSurfaceKHR surface, int w, int h);
	void destroy();
	void resize(int w, int h);
	void run();

private:
	void createSwapChain();
	
private:
	VulkanGraphicsView view;
	VkSurfaceKHR surface;

	int windowWidth, windowHeight;
	
	// Swapchain
	VkSwapchainKHR swapchain;
	VkFormat swapchainImageFormat;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;
	VkExtent2D swapchainExtent;

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
