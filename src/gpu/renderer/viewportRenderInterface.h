#ifndef viewportRenderInterface_h
#define viewportRenderInterface_h

#include <RmlUi/Core/Element.h>
#include <vulkan/vulkan.h>

#include "backend/circuitView/renderer/renderer.h"
#include "gpu/renderer/vulkanChunkRenderer.h"
#include "gpu/renderer/vulkanFrame.h"

struct WindowRenderingManager;

class ViewportRenderInterface : public Renderer {
public:
	ViewportRenderInterface(WindowRenderingManager* windowRenderer, Rml::Element* element);
	~ViewportRenderInterface();

	void initializeVulkan(VkRenderPass renderPass);
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
	WindowRenderingManager* windowRenderer;
	Rml::Element* element;

	// Vulkan
	std::unique_ptr<VulkanChunkRenderer> chunkRenderer = nullptr;
};

#endif
