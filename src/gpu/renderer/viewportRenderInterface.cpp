#include "viewportRenderInterface.h"

#include <glm/ext/matrix_clip_space.hpp>

#include "gpu/renderer/viewport/elements/elementRenderer.h"
#include "gpu/renderer/windowRenderer.h"
#include "viewport/sharedLogic/logicRenderingUtils.h"
#include "backend/selection.h"

ViewportRenderInterface::ViewportRenderInterface(VulkanDevice* device, Rml::Element* element)
	: element(element), chunker(device) {
}

ViewportRenderInterface::~ViewportRenderInterface() {
	if (linkedWindowRenderer != nullptr) linkedWindowRenderer->deregisterViewportRenderInterface(this);
}


void ViewportRenderInterface::linkToWindowRenderer(WindowRenderer* windowRenderer) {
	linkedWindowRenderer = windowRenderer;
	linkedWindowRenderer->registerViewportRenderInterface(this);
}

ViewportViewData ViewportRenderInterface::getViewData() {
	std::lock_guard<std::mutex> lock(viewMux);
	return viewData;
}

// ====================================== INTERFACE ==========================================

void ViewportRenderInterface::setCircuit(Circuit* circuit) {
	std::lock_guard<std::mutex> lock(circuitMux);
	
	this->circuit = circuit;
	chunker.setCircuit(circuit);
}

void ViewportRenderInterface::setEvaluator(std::shared_ptr<Evaluator> evaluator) {
	std::lock_guard<std::mutex> lock(evaluatorMux);
	this->evaluator = evaluator;
}

void ViewportRenderInterface::setAddress(const Address& address) {
	
}

void ViewportRenderInterface::updateView(ViewManager* viewManager) {
	std::lock_guard<std::mutex> lock(viewMux);
	
	// Update vulkan viewport
	viewData.viewport.x = element->GetAbsoluteOffset().x;
	viewData.viewport.y = element->GetAbsoluteOffset().y;
	viewData.viewport.width = element->GetBox().GetSize().x;
	viewData.viewport.height = element->GetBox().GetSize().y;
	viewData.viewport.minDepth = 0.0f;
	viewData.viewport.maxDepth = 1.0f;

	// Create view mat
	FPosition topLeft = viewManager->getTopLeft();
	FPosition bottomRight = viewManager->getBottomRight();
	viewData.viewportViewMat = glm::ortho(topLeft.x, bottomRight.x, topLeft.y, bottomRight.y);
	viewData.viewBounds = { topLeft, bottomRight };

	viewData.viewScale = viewManager->getViewScale();
}

void ViewportRenderInterface::updateCircuit(DifferenceSharedPtr diff) {
	chunker.updateCircuit(diff);
}

float ViewportRenderInterface::getLastFrameTimeMs() const {
	return 0.0f;
}

ElementID ViewportRenderInterface::addSelectionObjectElement(const SelectionObjectElement& selection) {
	std::lock_guard<std::mutex> lock(elementsMux);

	ElementID newElement = ++currentElementID;

	std::stack<SharedSelection> selectionsLeft;
	selectionsLeft.push(selection.selection);
	while (!selectionsLeft.empty()) {
		SharedSelection selectionObj = selectionsLeft.top();
		selectionsLeft.pop();

		// add cell selection no matter what
		SharedCellSelection cellSelection = selectionCast<CellSelection>(selectionObj);
		if (cellSelection) {
			BoxSelectionRenderData newBoxSelection;
			Position position = cellSelection->getPosition();

			newBoxSelection.topLeft = glm::vec2(position.x, position.y);
			newBoxSelection.size = glm::vec2(1.0f);
			if (selection.renderMode == SelectionObjectElement::RenderMode::SELECTION) newBoxSelection.state = BoxSelectionRenderData::Normal;
			else if (selection.renderMode == SelectionObjectElement::RenderMode::SELECTION_INVERTED) newBoxSelection.state = BoxSelectionRenderData::Inverted;
			else if (selection.renderMode == SelectionObjectElement::RenderMode::ARROWS) newBoxSelection.state = BoxSelectionRenderData::Special;

			boxSelections[newElement].push_back(newBoxSelection);
		}

		
		SharedDimensionalSelection dimensionalSelection = selectionCast<DimensionalSelection>(selectionObj);
		// if we're dimensional and stuff we gotta do stuff
		if (dimensionalSelection) {
			
			if (selection.renderMode != SelectionObjectElement::RenderMode::ARROWS) {
				// go through dimensions if we are normal
				for (int i = 0; i < dimensionalSelection->size(); i++) {
					selectionsLeft.push(dimensionalSelection->getSelection(i));
				}
			} else {
				// if we are arrows, do some fucked shit
				SharedProjectionSelection projectionSelection = selectionCast<ProjectionSelection>(selectionObj);
				if (projectionSelection) {
					// if we have projection, add some arrows

					// calculate height and origin
					SharedDimensionalSelection dSel = dimensionalSelection;
					Position origin; // this variable used to be called 'orgin' because Ben can't spell (or maybe it's a typo)
					unsigned int height = 0;
					while (dSel) {
						SharedSelection sel = dSel->getSelection(0);
						SharedCellSelection cSel = selectionCast<CellSelection>(sel);
						if (cSel) {
							origin = cSel->getPosition();
							break;
						}
						height++;
						dSel = selectionCast<DimensionalSelection>(sel);
					}
				
					selectionsLeft.push(dimensionalSelection->getSelection(0)); // no idea why we do this, I don't understand the selection system
				
					if (projectionSelection->size() == 1) {
						// add this kind of arrow
						arrows[newElement].push_back(ArrowRenderData(origin, origin, height));
					} else {
						// add a bunch of different kinds of arrows
						for (int i = 1; i < projectionSelection->size(); i++) {
							Position newOrigin = origin + projectionSelection->getStep();
							arrows[newElement].push_back(ArrowRenderData(origin, newOrigin, height));
							origin = newOrigin;
						}
					}
				} else {
					// we don't don't have projection, go through dimensions
					for (int i = 0; i < dimensionalSelection->size(); i++) {
						selectionsLeft.push(dimensionalSelection->getSelection(i));
					}
				}
			}
		}
	}

	return newElement;
}

ElementID ViewportRenderInterface::addSelectionElement(const SelectionElement& selection) {
	std::lock_guard<std::mutex> lock(elementsMux);

	ElementID newElement = ++currentElementID;

	// calculate ordered box
	FPosition topLeft = selection.topLeft.free();
	FPosition bottomRight = selection.bottomRight.free();
	if (selection.topLeft.x > selection.bottomRight.x) std::swap(topLeft.x, bottomRight.x);
	if (selection.topLeft.y > selection.bottomRight.y) std::swap(topLeft.y, bottomRight.y);

	BoxSelectionRenderData newBoxSelection;
	newBoxSelection.topLeft = glm::vec2(topLeft.x, topLeft.y);
	newBoxSelection.size = glm::vec2(bottomRight.x - topLeft.x + 1.0f, bottomRight.y - topLeft.y + 1.0f);
	newBoxSelection.state = selection.inverted ? BoxSelectionRenderData::Inverted : BoxSelectionRenderData::Normal;

	boxSelections[newElement].push_back(newBoxSelection);

	return newElement;
}

void ViewportRenderInterface::removeSelectionElement(ElementID selection) {
	std::lock_guard<std::mutex> lock(elementsMux);

	boxSelections.erase(selection);
	arrows.erase(selection);
}

std::vector<ArrowRenderData> ViewportRenderInterface::getArrows() {
	std::lock_guard<std::mutex> lock(elementsMux);
	
	std::vector<ArrowRenderData> returnArrows;
	returnArrows.reserve(arrows.size());

	for (const auto& arrow : arrows) {
		returnArrows.insert(returnArrows.end(), arrow.second.begin(), arrow.second.end());
	}

	return returnArrows;
}

std::vector<BoxSelectionRenderData> ViewportRenderInterface::getBoxSelections() {
	std::lock_guard<std::mutex> lock(elementsMux);

	std::vector<BoxSelectionRenderData> returnBoxSelections;
	returnBoxSelections.reserve(boxSelections.size());

	for (const auto& boxSelection : boxSelections) {
		returnBoxSelections.insert(returnBoxSelections.end(), boxSelection.second.begin(), boxSelection.second.end());
	}

	return returnBoxSelections;
}

ElementID ViewportRenderInterface::addBlockPreview(const BlockPreview& blockPreview) {
	std::lock_guard<std::mutex> lock(elementsMux);

	ElementID newElement = ++currentElementID;

	BlockPreviewRenderData newPreview;
	newPreview.position = glm::vec2(blockPreview.position.x, blockPreview.position.y);
	newPreview.rotation = blockPreview.rotation;
	{
		std::lock_guard<std::mutex> lock(circuitMux);
		Vector size(1.0f, 1.0f);
		if (circuit) size = circuit->getBlockContainer()->getBlockDataManager()->getBlockSize(blockPreview.type, blockPreview.rotation);
		newPreview.size = glm::vec2(size.dx, size.dy);
	}
	newPreview.type = blockPreview.type;

	// insert new block preview into map
	blockPreviews[newElement] = newPreview;

	return newElement;
}

void ViewportRenderInterface::removeBlockPreview(ElementID blockPreview) {
	std::lock_guard<std::mutex> lock(elementsMux);

	blockPreviews.erase(blockPreview);
}

std::vector<BlockPreviewRenderData> ViewportRenderInterface::getBlockPreviews() {
	std::lock_guard<std::mutex> lock(elementsMux);

	std::vector<BlockPreviewRenderData> returnBlockPreviews;
	returnBlockPreviews.reserve(blockPreviews.size());

	for (const auto& preview : blockPreviews) {
		returnBlockPreviews.push_back(preview.second);
	}

	return returnBlockPreviews;
}

ElementID ViewportRenderInterface::addConnectionPreview(const ConnectionPreview& connectionPreview) {
	std::lock_guard<std::mutex> lock(elementsMux);

	ElementID newElement = ++currentElementID;
	if (circuit) {
		std::lock_guard<std::mutex> lock(circuitMux);
		ConnectionPreviewRenderData newPreview;
		FPosition pointA = connectionPreview.output.free() + getOutputOffset(connectionPreview.output, circuit);
		FPosition pointB = connectionPreview.input.free() + getInputOffset(connectionPreview.input, circuit);
		newPreview.pointA = glm::vec2(pointA.x, pointA.y);
		newPreview.pointB = glm::vec2(pointB.x, pointB.y);
		connectionPreviews[newElement] = newPreview;
	}

	return newElement;
}

void ViewportRenderInterface::removeConnectionPreview(ElementID connectionPreview) {
	std::lock_guard<std::mutex> lock(elementsMux);

	connectionPreviews.erase(connectionPreview);
}

ElementID ViewportRenderInterface::addHalfConnectionPreview(const HalfConnectionPreview& halfConnectionPreview) {
	std::lock_guard<std::mutex> lock(elementsMux);

	ElementID newElement = ++currentElementID;
	if (circuit) {
		std::lock_guard<std::mutex> lock(circuitMux);
		ConnectionPreviewRenderData newPreview;

		FPosition pointA = halfConnectionPreview.output.free() + getOutputOffset(halfConnectionPreview.output, circuit);
		newPreview.pointA = glm::vec2(pointA.x, pointA.y);
		newPreview.pointB = glm::vec2(halfConnectionPreview.input.x, halfConnectionPreview.input.y);
		connectionPreviews[newElement] = newPreview;
	}

	return newElement;
}

void ViewportRenderInterface::removeHalfConnectionPreview(ElementID halfConnectionPreview) {
	std::lock_guard<std::mutex> lock(elementsMux);

	connectionPreviews.erase(halfConnectionPreview);
}

std::vector<ConnectionPreviewRenderData> ViewportRenderInterface::getConnectionPreviews() {
	std::lock_guard<std::mutex> lock(elementsMux);

	std::vector<ConnectionPreviewRenderData> returnConnectionPreviews;
	returnConnectionPreviews.reserve(connectionPreviews.size());

	for (const auto& preview : connectionPreviews) {
		returnConnectionPreviews.push_back(preview.second);
	}

	return returnConnectionPreviews;
}

void ViewportRenderInterface::spawnConfetti(FPosition start) {

}

