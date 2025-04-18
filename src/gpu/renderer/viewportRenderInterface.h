#ifndef viewportRenderInterface_h
#define viewportRenderInterface_h

#include <RmlUi/Core/Element.h>
#include <vulkan/vulkan.h>

#include "backend/circuitView/renderer/renderer.h"
#include "gpu/renderer/subrenderers/vulkanChunker.h"
#include "gpu/renderer/vulkanFrame.h"

struct WindowRenderer;

struct ViewportViewData {
	glm::mat4 viewportViewMat;
	std::pair<FPosition, FPosition> viewBounds;
	VkViewport viewport;
};

class ViewportRenderInterface : public Renderer {
public:
	ViewportRenderInterface(Rml::Element* element);
	~ViewportRenderInterface();

	void linkToWindowRenderer(WindowRenderer* windowRenderer);
	
	ViewportViewData getViewData();
	inline VulkanChunker& getChunker() { return chunker; }
	
	void render(VulkanFrameData& frame);
	
public:
	// main flow
	void setCircuit(Circuit* circuit) override;
	void setEvaluator(Evaluator* evaluator) override;
	void setAddress(const Address& address) override;

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
	WindowRenderer* linkedWindowRenderer = nullptr;
	Rml::Element* element;

	// Vulkan
	VulkanChunker chunker; // this should eventually probably be per circuit instead of per view

	// View data
	ViewportViewData viewData;
	std::mutex viewMux;
};

#endif
