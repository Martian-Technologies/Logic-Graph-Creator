// #include <iostream>

#include "moveTool.h"

bool MoveTool::click(const Event* event) {
	if (!circuit) return false;

	if (stage == 'o') {
		if (tensorStage == -1) {
			originPosition = lastPointerPosition;
			originSelection = std::make_shared<CellSelection>(originPosition);
			tensorStage = 0;
		} else if (tensorStage % 2 == 0) { // step
			step = lastPointerPosition - originPosition;
			if (step.manhattenlength() == 0) {
				originSelection = std::make_shared<ProjectionSelection>(originSelection, Vector(), 1);
				tensorStage += 2;
			} else {
				tensorStage++;
			}
		} else { // count
			float dis = step.length();
			float length = lastPointerFPosition.lengthAlongProjectToVec(originPosition.free() + FVector(0.5f, 0.5f), step.free());
			int count = Abs(round(length / dis)) + 1;
			originSelection = std::make_shared<ProjectionSelection>(originSelection, (length > 0) ? step : step * -1, count);
			tensorStage++;
		}
	} else {
		destination = lastPointerPosition;
		if (circuit->tryMoveBlocks(originSelection, destination - originPosition)) reset();
	}
	updateElements();
	return true;
}

bool MoveTool::unclick(const Event* event) {
	if (stage == 'o') {
		if (tensorStage == -1) return false;
		// undo orgin
		if (tensorStage == 0) {
			originSelection = nullptr;
			tensorStage = -1;
		} else if (tensorStage % 2 == 1) { // undo step
			tensorStage--;
		} else { // undo count
			SharedProjectionSelection projectionSelection = selectionCast<ProjectionSelection>(originSelection);
			if (projectionSelection->size() == 1) {
				tensorStage -= 2;
				originSelection = projectionSelection->getSelection(0);
			} else {
				step = projectionSelection->getStep();
				originSelection = projectionSelection->getSelection(0);
				tensorStage--;
			}
		}
	} else {
		stage = 'o';
	}
	updateElements();
	return true;
}

bool MoveTool::confirm(const Event* event) {
	if (stage == 'o') {
		if (tensorStage == -1) return false;
		if (tensorStage % 2 == 1) tensorStage--;
		stage = 'd';
		updateElements();
	}

	return false;
}

void MoveTool::updateElements() {
	if (!elementCreator.isSetup()) return;
	elementCreator.clear();
	if (!pointerInView) {
		if (tensorStage != -1) {
			if (tensorStage % 2 == 1) {
				elementCreator.addSelectionElement(SelectionObjectElement(
					std::make_shared<ProjectionSelection>(originSelection, step, 2),
					SelectionObjectElement::RenderMode::ARROWS
				));
			} else {
				elementCreator.addSelectionElement(SelectionObjectElement(
					originSelection, SelectionObjectElement::RenderMode::ARROWS
				));
			}
		}
		return;
	};

	if (stage == 'd') {
		elementCreator.addSelectionElement(SelectionObjectElement(originSelection, SelectionObjectElement::RenderMode::ARROWS));
		elementCreator.addSelectionElement(SelectionObjectElement(shiftSelection(originSelection, lastPointerPosition - originPosition)));
	} else {
		SharedSelection selection;
		if (tensorStage == -1) {
			selection = std::make_shared<CellSelection>(lastPointerPosition);
		} else if (tensorStage % 2 == 0) { // step
			step = lastPointerPosition - originPosition;
			if (step.manhattenlength() == 0) {
				selection = std::make_shared<ProjectionSelection>(originSelection, Vector(), 1);
			} else {
				selection = std::make_shared<ProjectionSelection>(originSelection, step, 2);
			}
		} else { // count
			float dis = step.length();
			float length = lastPointerFPosition.lengthAlongProjectToVec(originPosition.free() + FVector(0.5f, 0.5f), step.free());
			int count = Abs(round(length / dis)) + 1;
			selection = std::make_shared<ProjectionSelection>(originSelection, (length > 0) ? step : step * -1, count);
		}
		elementCreator.addSelectionElement(SelectionObjectElement(selection, SelectionObjectElement::RenderMode::ARROWS));
	}
}
