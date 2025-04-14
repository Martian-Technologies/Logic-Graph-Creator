#ifndef viewportRenderer_h
#define viewportRenderer_h

#include <RmlUi/Core/Element.h>
#include <vulkan/vulkan.h>

#include "backend/circuitView/renderer/renderer.h"
#include "gpu/renderer/vulkanChunkRenderer.h"
#include "gpu/renderer/vulkanFrame.h"

struct WindowRenderer;

struct ViewportViewData {
	glm::mat4 viewportViewMat;
	std::pair<FPosition, FPosition> viewBounds;
	VkViewport viewport;
};

class ViewportRenderer : public Renderer {
public:
	ViewportRenderer(WindowRenderer* windowRenderer, Rml::Element* element);
	~ViewportRenderer();

	void initializeVulkan(VkRenderPass renderPass);
	void destroyVulkan();
	void render(VulkanFrameData& frame);
	
public:
	// main flow
	void setCircuit(Circuit* circuit) override;
	void setEvaluator(Evaluator* evaluator) override;

	void updateView(ViewManager* viewManager) override;
	void updateCircuit(DifferenceSharedPtr diff) override;

	float getLastFrameTimeMs() const override;

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
	// From the UI Side
	WindowRenderer* windowRenderer;
	Rml::Element* element;

	// Vulkan
	std::unique_ptr<VulkanChunkRenderer> chunkRenderer = nullptr;

	ViewportViewData viewData;
	std::mutex viewMux;
};

#endif
