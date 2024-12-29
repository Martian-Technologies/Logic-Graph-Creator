#include <iostream>

#include <Qt>

#include "singleConnectTool.h"

bool SingleConnectTool::makeConnection(const Event* event) {
    if (!blockContainer) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;
    
    if (clicked) {
        if (!blockContainer->tryRemoveConnection(clickPosition, positionEvent->getPosition())) {
            blockContainer->tryCreateConnection(clickPosition, positionEvent->getPosition());
        }
        reset();
        return true;
    }
    else {
        if (!blockContainer->getBlockContainer()->getOutputConnectionEnd(positionEvent->getPosition()).has_value()) {
            return false;
        }
        
        clicked = true;
        clickPosition = positionEvent->getPosition();
        return true;
    }
}

bool SingleConnectTool::cancelConnection(const Event* event) {
    if (clicked) {
        reset();
        return true;
    }
    return false;
}

bool SingleConnectTool::pointerMove(const Event* event) {
    if (!blockContainer) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;
    
    updateElements(positionEvent->getPosition());
    
    return false;
}

bool SingleConnectTool::enterBlockView(const Event* event) {
    if (!blockContainer) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;

    updateElements(positionEvent->getPosition());
    
    return true;
}

bool SingleConnectTool::exitBlockView(const Event* event) {
    if (!blockContainer) return false;
    elementCreator.clear();
    return true;
}

void SingleConnectTool::updateElements(Position pointerPosition) {
    elementCreator.clear();
    
    
    if (clicked) {
        elementCreator.addConnectionPreview(clickPosition, pointerPosition);
        bool valid = blockContainer->getBlockContainer()->getInputConnectionEnd(pointerPosition).has_value();
        elementCreator.addSelectionElement(pointerPosition, !valid);
    }
    else {
        // TODO - change to use isvalid function
        bool valid = blockContainer->getBlockContainer()->getOutputConnectionEnd(pointerPosition).has_value();
        elementCreator.addSelectionElement(pointerPosition, !valid);
    }
}
