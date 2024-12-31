#include <iostream>

#include "tensorConnectTool.h"

bool TensorConnectTool::click(const Event* event) {
    if (!blockContainer) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;

    if (makingOutput) {
        // orgin
        if (outputStage == -1) {
            outputPosition = positionEvent->getPosition();
            outputSelection = std::make_shared<CellSelection>(outputPosition);
            outputStage = 0;
        } else if (outputStage % 2 == 0) { // step
            step = positionEvent->getPosition();
            if (step == outputPosition) {
                outputSelection = std::make_shared<ProjectionSelection>(outputSelection, Position(), 1);
                outputStage += 2;
            } else {
                outputStage++;
            }
        } else { // count
            int dis = outputPosition.distanceTo(step);
            float length = positionEvent->getFPosition().lengthAlongProjectToVec(outputPosition.free() + FPosition(0.5f, 0.5f), step.free() + FPosition(0.5f, 0.5f));
            int count = Abs(round(length / dis)) + 1;
            outputSelection = std::make_shared<ProjectionSelection>(outputSelection, (length > 0) ? step - outputPosition : outputPosition - step, count);
            outputStage++;
        }
    } else {
        // orgin
        if (inputStage == -1) {
            inputPosition = positionEvent->getPosition();
            inputSelection = std::make_shared<CellSelection>(inputPosition);
            inputStage = 0;
        } else if (inputStage % 2 == 0) { // step
            step = positionEvent->getPosition();
            if (step == inputPosition) {
                inputSelection = std::make_shared<ProjectionSelection>(inputSelection, Position(), 1);
                inputStage += 2;
            } else {
                inputStage++;
            }
        } else { // count
            int dis = inputPosition.distanceTo(step);
            float length = positionEvent->getFPosition().lengthAlongProjectToVec(inputPosition.free() + FPosition(0.5f, 0.5f), step.free() + FPosition(0.5f, 0.5f));
            int count = Abs(round(length / dis)) + 1;
            inputSelection = std::make_shared<ProjectionSelection>(inputSelection, (length > 0) ? step - inputPosition : inputPosition - step, count);
            inputStage++;
        }
    }
    return true;
}

bool TensorConnectTool::unclick(const Event* event) {
    if (!blockContainer) return false;
    if (makingOutput) {
        if (outputStage == -1) return false;
        // undo orgin
        if (outputStage == 0) {
            outputSelection = nullptr;
            outputStage = -1;
        } else if (outputStage % 2 == 1) { // undo step
            outputStage--;
        } else { // undo count
            SharedProjectionSelection projectionSelection = selectionCast<ProjectionSelection>(outputSelection);
            if (projectionSelection->size() == 1) {
                outputStage -= 2;
                outputSelection = projectionSelection->getSelection(0);
            } else {
                step = projectionSelection->getStep() + outputPosition;
                outputSelection = projectionSelection->getSelection(0);
                outputStage--;
            }
        }
    } else {
        if (inputStage == -1) {
            makingOutput = true;
        } else if (inputStage == 0) {// undo orgin
            inputSelection = nullptr;
            inputStage = -1;
        } else if (inputStage % 2 == 1) { // undo step
            inputStage--;
        } else { // undo count
            SharedProjectionSelection projectionSelection = selectionCast<ProjectionSelection>(inputSelection);
            if (projectionSelection->size() == 1) {
                inputStage -= 2;
                inputSelection = projectionSelection->getSelection(0);
            } else {
                step = projectionSelection->getStep() + inputPosition;
                inputSelection = projectionSelection->getSelection(0);
                inputStage--;
            }
        }
    }
    return true;
}

bool TensorConnectTool::confirm(const Event* event) {
    if (!blockContainer) return false;
    if (makingOutput) {
        if (outputStage == -1) return false;
        makingOutput = false;
        inputStage = -1;
        return true;
    } else {
        if (inputStage != outputStage) return false;
        std::cout << "DONE" << std::endl;
        return true;
    }

    return false;
}

bool TensorConnectTool::pointerMove(const Event* event) {
    if (!blockContainer) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;

    if (makingOutput) {
        // orgin
        if (outputStage == -1) {
            updateElements(std::make_shared<CellSelection>(positionEvent->getPosition()));
        } else if (outputStage % 2 == 0) { // step
            step = positionEvent->getPosition();
            if (step == outputPosition) {
                updateElements(std::make_shared<ProjectionSelection>(outputSelection, Position(), 1));
            } else {
                updateElements(std::make_shared<ProjectionSelection>(outputSelection, step - outputPosition, 2));
            }
        } else { // count
            int dis = outputPosition.distanceTo(step);
            float length = positionEvent->getFPosition().lengthAlongProjectToVec(outputPosition.free() + FPosition(0.5f, 0.5f), step.free() + FPosition(0.5f, 0.5f));
            int count = Abs(round(length / dis)) + 1;
            updateElements(std::make_shared<ProjectionSelection>(outputSelection, (length > 0) ? step - outputPosition : outputPosition - step, count));
        }
    } else {
        // orgin
        if (inputStage == -1) {
            updateElements(std::make_shared<CellSelection>(positionEvent->getPosition()));
        } else if (inputStage % 2 == 0) { // step
            step = positionEvent->getPosition();
            if (step == inputPosition) {
                updateElements(std::make_shared<ProjectionSelection>(inputSelection, Position(), 1));
            } else {
                updateElements(std::make_shared<ProjectionSelection>(inputSelection, step - inputPosition, 2));
            }
        } else { // count
            int dis = inputPosition.distanceTo(step);
            float length = positionEvent->getFPosition().lengthAlongProjectToVec(inputPosition.free() + FPosition(0.5f, 0.5f), step.free() + FPosition(0.5f, 0.5f));
            int count = Abs(round(length / dis)) + 1;
            updateElements(std::make_shared<ProjectionSelection>(inputSelection, (length > 0) ? step - inputPosition : inputPosition - step, count));
        }
    }
    return true;
}

void TensorConnectTool::updateElements(SharedSelection selection) {
    elementCreator.clear();
    if (makingOutput) {
        elementCreator.addSelectionElement(selection);
    } else {
        elementCreator.addSelectionElement(selection);
        elementCreator.addSelectionElement(outputSelection);
    }
}
