// #include <iostream>

#include "moveTool.h"

bool MoveTool::click(const Event* event) {
    if (!blockContainer) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;

    if (stage == 'o') {
        if (tensorStage == -1) {
            originPosition = positionEvent->getPosition();
            originSelection = std::make_shared<CellSelection>(originPosition);
            tensorStage = 0;
        } else if (tensorStage % 2 == 0) { // step
            step = positionEvent->getPosition();
            if (step == originPosition) {
                originSelection = std::make_shared<ProjectionSelection>(originSelection, Position(), 1);
                tensorStage += 2;
            } else {
                tensorStage++;
            }
        } else { // count
            int dis = originPosition.distanceTo(step);
            float length = positionEvent->getFPosition().lengthAlongProjectToVec(originPosition.free() + FPosition(0.5f, 0.5f), step.free() + FPosition(0.5f, 0.5f));
            int count = Abs(round(length / dis)) + 1;
            originSelection = std::make_shared<ProjectionSelection>(originSelection, (length > 0) ? step - originPosition : originPosition - step, count);
            tensorStage++;
        }
    } else {
        destination = positionEvent->getPosition();
        if (blockContainer->tryMoveBlocks(originSelection, destination - originPosition)) reset();
    }
    updateElements();
    return true;
}

bool MoveTool::unclick(const Event* event) {
    if (!blockContainer) return false;
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
                step = projectionSelection->getStep() + originPosition;
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
    if (!blockContainer) return false;
    if (stage == 'o') {
        if (tensorStage == -1) return false;
        if (tensorStage % 2 == 1) tensorStage--;
        stage = 'd';
    }

    return false;
}

bool MoveTool::pointerMove(const Event* event) {
    if (!blockContainer) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;
    pointer = positionEvent->getFPosition();
    updateElements();
    return true;
}

void MoveTool::updateElements() {
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
            step = pointer.snap();
            if (step == originPosition) {
                selection = std::make_shared<ProjectionSelection>(originSelection, Position(), 1);
            } else {
                selection = std::make_shared<ProjectionSelection>(originSelection, step - originPosition, 2);
            }
        } else { // count
            int dis = originPosition.distanceTo(step);
            float length = pointer.lengthAlongProjectToVec(originPosition.free() + FPosition(0.5f, 0.5f), step.free() + FPosition(0.5f, 0.5f));
            int count = Abs(round(length / dis)) + 1;
            selection = std::make_shared<ProjectionSelection>(originSelection, (length > 0) ? step - originPosition : originPosition - step, count);
        }
        elementCreator.addSelectionElement(SelectionObjectElement(selection, SelectionObjectElement::RenderMode::ARROWS));
    }
}
