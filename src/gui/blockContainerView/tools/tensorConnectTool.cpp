// #include <iostream>

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
        if (outputStage <= inputStage) return false;
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
                SharedDimensionalSelection outputSimilarSelection = selectionCast<DimensionalSelection>(outputSelection);
                for (int i = outputStage / 2 - inputStage / 2 - 1; i > 0; i--) {
                    outputSimilarSelection = selectionCast<DimensionalSelection>(outputSimilarSelection->getSelection(0));
                }
                if (outputSimilarSelection->size() == 1) {
                    inputStage++;
                } else {
                    inputSelection = std::make_shared<ProjectionSelection>(inputSelection, step - inputPosition, outputSimilarSelection->size());
                    inputStage += 2;
                }
            }
        } else { // count
            int dis = inputPosition.distanceTo(step);
            float length = positionEvent->getFPosition().lengthAlongProjectToVec(inputPosition.free() + FPosition(0.5f, 0.5f), step.free() + FPosition(0.5f, 0.5f));
            int count = Abs(round(length / dis)) + 1;
            inputSelection = std::make_shared<ProjectionSelection>(inputSelection, (length > 0) ? step - inputPosition : inputPosition - step, count);
            inputStage++;
        }
    }
    updateElements();
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
                SharedDimensionalSelection outputSimilarSelection = selectionCast<DimensionalSelection>(outputSelection);
                for (int i = outputStage / 2 - inputStage / 2 - 2; i > 0; i--) {
                    outputSimilarSelection = selectionCast<DimensionalSelection>(outputSimilarSelection->getSelection(0));
                }
                if (outputSimilarSelection->size() == 1) {
                    step = projectionSelection->getStep() + inputPosition;
                    inputSelection = projectionSelection->getSelection(0);
                    inputStage--;
                } else {
                    inputSelection = projectionSelection->getSelection(0);
                    inputStage -= 2;
                }
            }
        }
    }
    updateElements();
    return true;
}

bool TensorConnectTool::confirm(const Event* event) {
    if (!blockContainer) return false;
    if (makingOutput) {
        if (outputStage == -1) return false;
        if (outputStage % 2 == 1) outputStage--;
        makingOutput = false;
        inputStage = -1;
    } else {
        if (inputStage != outputStage) return false;
        if (!sameSelectionShape(inputSelection, outputSelection)) return false;
        // std::cout << "DONE" << std::endl;
        blockContainer->tryCreateConnection(outputSelection, inputSelection);
        reset();
    }
    updateElements();
    return true;
}

bool TensorConnectTool::pointerMove(const Event* event) {
    if (!blockContainer) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;
    pointer = positionEvent->getFPosition();
    updateElements();
    return true;
}

void TensorConnectTool::updateElements() {
    if (!elementCreator.isSetup()) return;
    elementCreator.clear();
    SharedSelection selection;
    if (makingOutput) {
        // orgin
        if (outputStage == -1) {
            selection = std::make_shared<CellSelection>(pointer.snap());
        } else if (outputStage % 2 == 0) { // step
            step = pointer.snap();
            if (step == outputPosition) {
                selection = std::make_shared<ProjectionSelection>(outputSelection, Position(), 1);
            } else {
                selection = std::make_shared<ProjectionSelection>(outputSelection, step - outputPosition, 2);
            }
        } else { // count
            int dis = outputPosition.distanceTo(step);
            float length = pointer.lengthAlongProjectToVec(outputPosition.free() + FPosition(0.5f, 0.5f), step.free() + FPosition(0.5f, 0.5f));
            int count = Abs(round(length / dis)) + 1;
            selection = std::make_shared<ProjectionSelection>(outputSelection, (length > 0) ? step - outputPosition : outputPosition - step, count);
        }
        elementCreator.addSelectionElement(SelectionObjectElement(selection, SelectionObjectElement::RenderMode::ARROWS));
    } else {
        // orgin
        if (outputStage <= inputStage) {
            elementCreator.addSelectionElement(SelectionObjectElement(outputSelection, SelectionObjectElement::RenderMode::ARROWS));
            elementCreator.addSelectionElement(SelectionObjectElement(inputSelection, SelectionObjectElement::RenderMode::ARROWS));
            return;
        }
        if (inputStage == -1) {
            selection = std::make_shared<CellSelection>(pointer.snap());
        } else if (inputStage % 2 == 0) { // step
            step = pointer.snap();
            if (step == inputPosition) {
                selection = std::make_shared<ProjectionSelection>(inputSelection, Position(), 1);
            } else {
                SharedDimensionalSelection outputSimilarSelection = selectionCast<DimensionalSelection>(outputSelection);
                for (int i = outputStage / 2 - inputStage / 2 - 1; i > 0; i--) {
                    outputSimilarSelection = selectionCast<DimensionalSelection>(outputSimilarSelection->getSelection(0));
                }
                if (outputSimilarSelection->size() == 1) {
                    selection = std::make_shared<ProjectionSelection>(inputSelection, step - inputPosition, 2);
                } else {
                    selection = std::make_shared<ProjectionSelection>(inputSelection, step - inputPosition, outputSimilarSelection->size());
                }

            }
        } else { // count
            int dis = inputPosition.distanceTo(step);
            float length = pointer.lengthAlongProjectToVec(inputPosition.free() + FPosition(0.5f, 0.5f), step.free() + FPosition(0.5f, 0.5f));
            int count = Abs(round(length / dis)) + 1;
            selection = std::make_shared<ProjectionSelection>(inputSelection, (length > 0) ? step - inputPosition : inputPosition - step, count);
        }
        elementCreator.addSelectionElement(SelectionObjectElement(outputSelection, SelectionObjectElement::RenderMode::ARROWS));
        elementCreator.addSelectionElement(SelectionObjectElement(selection, SelectionObjectElement::RenderMode::ARROWS));
    }
}
