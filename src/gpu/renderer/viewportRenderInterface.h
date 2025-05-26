#ifndef viewportRenderInterface_h
#define viewportRenderInterface_h

#include <RmlUi/Core/Element.h>
#include <glm/ext/matrix_float4x4.hpp>

#include "backend/circuitView/renderer/renderer.h"
#include "gpu/renderer/viewport/chunking/vulkanChunker.h"

#include "gpu/renderer/viewport/elements/elementRenderer.h"

struct WindowRenderer;

struct ViewportViewData {
	glm::mat4 viewportViewMat;
	std::pair<FPosition, FPosition> viewBounds;
	float viewScale;
	VkViewport viewport;
};

class ViewportRenderInterface : public Renderer {
public:
	ViewportRenderInterface(VulkanDevice* device, Rml::Element* element);
	~ViewportRenderInterface();

	void linkToWindowRenderer(WindowRenderer* windowRenderer);
	
	ViewportViewData getViewData();
	inline bool hasCircuit() { std::lock_guard<std::mutex> lock(circuitMux); return circuit != nullptr; }
	inline VulkanChunker& getChunker() { return chunker; }
	inline std::shared_ptr<Evaluator> getEvaluator() { std::lock_guard<std::mutex> lock(evaluatorMux); return evaluator; }

	std::vector<BlockPreviewRenderData> getBlockPreviews();
	std::vector<BoxSelectionRenderData> getBoxSelections();
	std::vector<ConnectionPreviewRenderData> getConnectionPreviews();
	
public:
	// main flow
	void setCircuit(Circuit* circuit) override;
	void setEvaluator(std::shared_ptr<Evaluator> evaluator) override;
	void setAddress(const Address& address) override;

	void updateView(ViewManager* viewManager) override;
	void updateCircuit(DifferenceSharedPtr diff) override;

	float getLastFrameTimeMs() const override;

private:
	// elements
	ElementID addSelectionObjectElement(const SelectionObjectElement& selection) override;
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
	Rml::Element* element;
	WindowRenderer* linkedWindowRenderer = nullptr;
	
	std::shared_ptr<Evaluator> evaluator = nullptr;
	std::mutex evaluatorMux;
	Circuit* circuit = nullptr;
	std::mutex circuitMux;

	// Vulkan
	VulkanChunker chunker; // this should eventually probably be per circuit instead of per view

	// Elements
	ElementID currentElementID = 0;
	std::unordered_map<ElementID, BlockPreviewRenderData> blockPreviews;
	std::unordered_map<ElementID, std::vector<BoxSelectionRenderData>> boxSelections;
	std::unordered_map<ElementID, ConnectionPreviewRenderData> connectionPreviews;
	std::mutex elementsMux;

	// View data
	ViewportViewData viewData;
	std::mutex viewMux;
};

#endif
