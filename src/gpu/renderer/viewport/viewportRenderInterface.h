#ifndef viewportRenderInterface_h
#define viewportRenderInterface_h

#include <glm/ext/matrix_float4x4.hpp>

#include "gui/viewPortManager/circuitView/renderer/renderer.h"
#include "elements/elementRenderer.h"
#include "logic/chunking/vulkanChunker.h"
#include "gpu/mainRenderer.h"

namespace Rml { class Element; }

struct WindowRenderer;

struct ViewportViewData {
	glm::mat4 viewportViewMat;
	std::pair<FPosition, FPosition> viewBounds;
	float viewScale;
	VkViewport viewport;
};

class ViewportRenderInterface : public CircuitViewRenderer {
public:
	ViewportRenderInterface(ViewportID ViewportID, Rml::Element* element);
	~ViewportRenderInterface();

	ViewportViewData getViewData();
	inline bool hasCircuit() {
		std::lock_guard<std::mutex> lock(circuitMux);
		return circuit != nullptr;
	}
	inline VulkanChunker& getChunker() { return chunker; }
	inline std::shared_ptr<Evaluator> getEvaluator() {
		std::lock_guard<std::mutex> lock(evaluatorMux);
		return evaluator;
	}
	inline const Address& getAddress() {
		std::lock_guard<std::mutex> lock(addressMux);
		return address;
	}

	std::vector<BlockPreviewRenderData> getBlockPreviews();
	std::vector<BoxSelectionRenderData> getBoxSelections();
	std::vector<ConnectionPreviewRenderData> getConnectionPreviews();
	std::vector<ArrowRenderData> getArrows();

public:
	// main flow
	void setCircuit(Circuit* circuit) override final;
	void setEvaluator(std::shared_ptr<Evaluator> evaluator) override final;
	void setAddress(const Address& address) override final;

	void updateView(ViewManager* viewManager) override final;

	float getLastFrameTimeMs() const override final;

private:
	// elements
	ElementID addSelectionObjectElement(const SelectionObjectElement& selection) override final;
	ElementID addSelectionElement(const SelectionElement& selection) override final;
	void removeSelectionElement(ElementID selection) override final;

	ElementID addBlockPreview(BlockPreview&& blockPreview) override final;
	void shiftBlockPreview(ElementID id, Vector shift) override final;
	void removeBlockPreview(ElementID blockPreview) override final;

	ElementID addConnectionPreview(const ConnectionPreview& connectionPreview) override final;
	void removeConnectionPreview(ElementID connectionPreview) override final;

	ElementID addHalfConnectionPreview(const HalfConnectionPreview& halfConnectionPreview) override final;
	void removeHalfConnectionPreview(ElementID halfConnectionPreview) override final;

	void spawnConfetti(FPosition start) override final;

private:
	// From the UI Side
	Rml::Element* element;
	WindowRenderer* linkedWindowRenderer = nullptr;

	std::shared_ptr<Evaluator> evaluator = nullptr;
	std::mutex evaluatorMux;
	Circuit* circuit = nullptr;
	std::mutex circuitMux;
	Address address;
	std::mutex addressMux;

	// Vulkan
	VulkanChunker chunker; // this should eventually probably be per circuit instead of per view
	std::optional<CircuitRenderManager> renderManager;

	// Elements
	ElementID currentElementID = 0;
	std::unordered_multimap<ElementID, BlockPreviewRenderData> blockPreviews;
	std::unordered_map<ElementID, std::vector<BoxSelectionRenderData>> boxSelections;
	std::unordered_map<ElementID, ConnectionPreviewRenderData> connectionPreviews;
	std::unordered_map<ElementID, std::vector<ArrowRenderData>> arrows;
	std::mutex elementsMux;

	// View data
	ViewportViewData viewData;
	std::mutex viewMux;
};

#endif
