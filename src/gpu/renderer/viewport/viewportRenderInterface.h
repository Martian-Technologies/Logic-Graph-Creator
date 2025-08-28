#ifndef viewportRenderInterface_h
#define viewportRenderInterface_h

#include <glm/ext/matrix_float4x4.hpp>

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

class ViewportRenderInterface {
public:
	ViewportRenderInterface(VulkanDevice* device, Rml::Element* element, WindowRenderer* windowRenderer);
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
	void setCircuit(Circuit* circuit) ;
	void setEvaluator(std::shared_ptr<Evaluator> evaluator) ;
	void setAddress(const Address& address) ;

	// void updateView(ViewManager* viewManager) ;

	float getLastFrameTimeMs() const ;

private:
	// elements
	ElementID addSelectionObjectElement(const SelectionObjectElement& selection) ;
	ElementID addSelectionElement(const SelectionElement& selection) ;
	void removeSelectionElement(ElementID selection) ;

	ElementID addBlockPreview(BlockPreview&& blockPreview) ;
	void shiftBlockPreview(ElementID id, Vector shift) ;
	void removeBlockPreview(ElementID blockPreview) ;

	ElementID addConnectionPreview(const ConnectionPreview& connectionPreview) ;
	void removeConnectionPreview(ElementID connectionPreview) ;

	ElementID addHalfConnectionPreview(const HalfConnectionPreview& halfConnectionPreview) ;
	void removeHalfConnectionPreview(ElementID halfConnectionPreview) ;

	void spawnConfetti(FPosition start) ;

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
