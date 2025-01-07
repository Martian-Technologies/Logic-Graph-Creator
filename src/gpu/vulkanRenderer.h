#ifndef vulkanRenderer_h
#define vulkanRenderer_h

#include "gui/blockContainerView/renderer/renderer.h"

#include "gpu/vulkanManager.h"

class VulkanRenderer : public Renderer {
public:
	void initialize(VulkanGraphicsView view, VkSurfaceKHR surface);
	void resize(int w, int h);
	void run();
	
	// updating
	void setBlockContainer(BlockContainerWrapper* blockContainer) override;
	void setEvaluator(Evaluator* evaluator) override;

	void updateView(ViewManager* viewManager) override;
	virtual void updateBlockContainer(DifferenceSharedPtr diff) override;

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
	VulkanGraphicsView view;
	float lastFrameTime = 0.0f;
};

#endif
