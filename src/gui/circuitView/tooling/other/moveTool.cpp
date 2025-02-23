// #include <iostream>

#include "moveTool.h"

bool MoveTool::click(const Event* event) {
	if (!circuit) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;

	if (stage == 'o') {
		if (tensorStage == -1) {
			originPosition = positionEvent->getPosition();
			originSelection = std::make_shared<CellSelection>(originPosition);
			tensorStage = 0;
		} else if (tensorStage % 2 == 0) { // step
			step = positionEvent->getPosition() - originPosition;
			if (step.manhattenlength() == 0) {
				originSelection = std::make_shared<ProjectionSelection>(originSelection, Vector(), 1);
				tensorStage += 2;
			} else {
				tensorStage++;
			}
		} else { // count
			float dis = step.length();
			float length = positionEvent->getFPosition().lengthAlongProjectToVec(originPosition.free() + FVector(0.5f, 0.5f), step.free());
			int count = Abs(round(length / dis)) + 1;
			originSelection = std::make_shared<ProjectionSelection>(originSelection, (length > 0) ? step : step * -1, count);
			tensorStage++;
		}
	} else {
		destination = positionEvent->getPosition();
		if (circuit->tryMoveBlocks(originSelection, destination - originPosition)) reset();
	}
	updateElements();
	return true;
}

bool MoveTool::unclick(const Event* event) {
	if (!circuit) return false;
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
		return true;
	} else {
		stage = 'o';
	}
	updateElements();
	return true;
}

bool MoveTool::confirm(const Event* event) {
	if (!circuit) return false;
	if (stage == 'o') {
		if (tensorStage == -1) return false;
		if (tensorStage % 2 == 1) tensorStage--;
		stage = 'd';
	}

	return false;
}

bool MoveTool::pointerMove(const Event* event) {
	if (!circuit) return false;
	const PositionEvent* positionEvent = event->cast<PositionEvent>();
	if (!positionEvent) return false;
	pointer = positionEvent->getFPosition();
	updateElements();
	return false;
}

void MoveTool::updateElements() {
	if (!elementCreator.isSetup()) return;
	if (!elementCreator.isSetup()) return;
	elementCreator.clear();
	if (stage == 'd') {
		elementCreator.addSelectionElement(SelectionObjectElement(originSelection, SelectionObjectElement::RenderMode::ARROWS));
		elementCreator.addSelectionElement(SelectionObjectElement(shiftSelection(originSelection, pointer.snap() - originPosition)));
	} else {
		SharedSelection selection;
		if (tensorStage == -1) {
			selection = std::make_shared<CellSelection>(pointer.snap());
		} else if (tensorStage % 2 == 0) { // step
			step = pointer.snap() - originPosition;
			if (step.manhattenlength() == 0) {
				selection = std::make_shared<ProjectionSelection>(originSelection, Vector(), 1);
			} else {
				selection = std::make_shared<ProjectionSelection>(originSelection, step, 2);
			}
		} else { // count
			float dis = step.length();
			float length = pointer.lengthAlongProjectToVec(originPosition.free() + FVector(0.5f, 0.5f), step.free());
			int count = Abs(round(length / dis)) + 1;
			selection = std::make_shared<ProjectionSelection>(originSelection, (length > 0) ? step : step * -1, count);
		}
		elementCreator.addSelectionElement(SelectionObjectElement(selection, SelectionObjectElement::RenderMode::ARROWS));
	}
}
