#include "pasteTool.h"
#include "backend/backend.h"

void PasteTool::activate() {
	CircuitTool::activate();
	registerFunction("Tool Primary Activate", std::bind(&PasteTool::place, this, std::placeholders::_1));
}

bool PasteTool::validatePlacement() const {
	SharedCopiedBlocks copiedBlocks = circuitView->getBackend()->getClipboard();
	if (!copiedBlocks) return false;

	Vector totalOffset = Vector(lastPointerPosition.x, lastPointerPosition.y) + (Position() - copiedBlocks->getMinPosition());

	for (const CopiedBlocks::CopiedBlockData& block : copiedBlocks->getCopiedBlocks()) {
		Position testPos = block.position + totalOffset;
		if (circuit->getBlockContainer()->checkCollision(testPos)) {
			return false;
		}
	}
	return true;
}

bool PasteTool::place(const Event* event) {
	if (!validatePlacement()) { return true; }

	SharedCopiedBlocks copiedBlocks = circuitView->getBackend()->getClipboard();
	circuit->tryInsertCopiedBlocks(copiedBlocks, lastPointerPosition);

	return true;
}

// Preview is only shown for the primary parsed circuit, not the dependencies that will be created in a different circuit
void PasteTool::updateElements() {
	if (!elementCreator.isSetup()) return;
	elementCreator.clear();
	if (!pointerInView) return;
	
	SharedCopiedBlocks copiedBlocks = circuitView->getBackend()->getClipboard();
	if (!copiedBlocks) return;

	Vector totalOffset = Vector(lastPointerPosition.x, lastPointerPosition.y) + (Position() - copiedBlocks->getMinPosition());

	for (const CopiedBlocks::CopiedBlockData& block : copiedBlocks->getCopiedBlocks()) {
		elementCreator.addBlockPreview(BlockPreview(block.blockType, block.position + totalOffset, block.rotation));
	}
}
