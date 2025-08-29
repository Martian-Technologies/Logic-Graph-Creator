#ifndef viewportRenderInterface_h
#define viewportRenderInterface_h

#include <glm/ext/matrix_float4x4.hpp>

#include "elements/elementRenderer.h"
#include "logic/chunking/vulkanChunker.h"
#include "gpu/mainRendererDefs.h"

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
	ViewportRenderInterface(WindowID windowID, VulkanDevice* device, Rml::Element* element, WindowRenderer* windowRenderer);
	~ViewportRenderInterface();

	ViewportViewData getViewData();
	inline bool hasCircuit() {
		std::lock_guard<std::mutex> lock(circuitMux);
		return circuit != nullptr;
	}
	inline VulkanChunker& getChunker() { return chunker; }
	inline Evaluator* getEvaluator() {
		std::lock_guard<std::mutex> lock(evaluatorMux);
		return evaluator;
	}
	inline const Address& getAddress() {
		std::lock_guard<std::mutex> lock(addressMux);
		return address;
	}
	inline WindowID getWindowID() const { return windowID; }

	std::vector<BlockPreviewRenderData> getBlockPreviews();
	std::vector<BoxSelectionRenderData> getBoxSelections();
	std::vector<ConnectionPreviewRenderData> getConnectionPreviews();
	std::vector<ArrowRenderData> getArrows();

	// main flow
	void setCircuit(Circuit* circuit) ;
	void setEvaluator(Evaluator* evaluator) ;
	void setAddress(const Address& address) ;

	void updateViewFrame(glm::vec2 origin, glm::vec2 size);
	void updateView(FPosition topLeft, FPosition bottomRight);

	float getLastFrameTimeMs() const ;

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

	Evaluator* evaluator = nullptr;
	std::mutex evaluatorMux;
	Circuit* circuit = nullptr;
	std::mutex circuitMux;
	Address address;
	std::mutex addressMux;

	
	// Vulkan
	WindowID windowID;
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
