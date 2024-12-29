#include <iostream>

#include <Qt>

#include "../../gridGUI/effects/cellSelectionEffect.h"
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
    clicked = true;
    clickPosition = positionEvent->getPosition();
    return true;
    return false;
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
    
    elementCreator.clear();
    elementCreator.addSelectionElement(positionEvent->getPosition());
    if (clicked) {
        elementCreator.addConnectionPreview(clickPosition, positionEvent->getPosition());
    }
    return false;
}

bool SingleConnectTool::enterBlockView(const Event* event) {
    if (!blockContainer) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;
    
    elementCreator.clear();
    elementCreator.addSelectionElement(positionEvent->getPosition());
    if (clicked) {
        elementCreator.addConnectionPreview(clickPosition, positionEvent->getPosition());
    }
    return true;
}

bool SingleConnectTool::exitBlockView(const Event* event) {
    if (!blockContainer) return false;
    elementCreator.clear();
    return true;
}
