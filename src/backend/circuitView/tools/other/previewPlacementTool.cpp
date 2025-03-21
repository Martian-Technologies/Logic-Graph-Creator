#include "previewPlacementTool.h"
#include "backend/backend.h"
#include "backend/circuitView/renderer/elementCreator.h"

// Preview is only shown for the primary parsed circuit, not the dependencies that will be created in a different circuit
void PreviewPlacementTool::updateElements() {
	if (!elementCreator.isSetup()) return;
	elementCreator.clear();
	
	if (!active || !pointerInView || !parsedCircuit) return;


	// note that getMinPos will return an FVector but will have valid integer coordinates.
	Vector totalOffset = (parsedCircuit->getMinPos() * -1) + Vector(lastPointerPosition.x, lastPointerPosition.y);
	//bool isValid = validatePlacement(); // possible change preview colors if invalid

	// only displays block previews, not connections
	for (const auto& [id, block] : parsedCircuit->getBlocks()) {
		Position displayPos = block.pos.snap() + totalOffset;
		elementCreator.addBlockPreview(BlockPreview(block.type, displayPos, block.rotation));
	}
}

bool PreviewPlacementTool::validatePlacement() const {
	Vector totalOffset = (parsedCircuit->getMinPos() * -1) + Vector(lastPointerPosition.x, lastPointerPosition.y);

	for (const auto& [id, block] : parsedCircuit->getBlocks()) {
		Position testPos = block.pos.snap() + totalOffset;
		if (circuit->getBlockContainer()->checkCollision(testPos)) {
			return false;
		}
	}
	return true;
}

// Places the primary parsed circuit in the current circuit.
// Places all dependencies on their own circuits.
bool PreviewPlacementTool::commitPlacement(const Event* event) {
	if (!active) return false;

	if (!validatePlacement()) {
		logError("Collision Detected. Cannot place circuit in occupied positions");
		return true;
	}
	circuit->tryInsertParsedCircuit(*parsedCircuit, lastPointerPosition);

	clearPreview();
	active = false;
	return true;
}

bool PreviewPlacementTool::cancelPlacement(const Event* event) {
	if (!active) return false;

	clearPreview();
	active = false;
	return true;
}

void PreviewPlacementTool::clearPreview() {
	elementCreator.clear();
}
