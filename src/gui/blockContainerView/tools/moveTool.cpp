// #include <iostream>

#include "moveTool.h"

bool MoveTool::click(const Event* event) {
    if (!blockContainer) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;

    switch (stage) {
    case 'o':
        origin = positionEvent->getPosition();
        originSelection = std::make_shared<CellSelection>(origin);
        stage = 'd';
        break;
    case 'd':
        destination = positionEvent->getPosition();
        blockContainer->tryMoveBlock(origin, destination);
        reset();
        break;
    }
    updateElements();
    return true;
}

bool MoveTool::unclick(const Event* event) {
    if (!blockContainer) return false;
    switch (stage) {
    case 'o':
        return false;
    case 'd':
        stage = 'o';
        break;
    }
    updateElements();
    return true;
}

bool MoveTool::confirm(const Event* event) {
    if (!blockContainer) return false;
    if (stage == 'o') {
        
    }

    return false;
}

bool MoveTool::pointerMove(const Event* event) {
    if (!blockContainer) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;
    pointer = positionEvent->getPosition();
    updateElements();
    return true;
}

void MoveTool::updateElements() {
    if (!elementCreator.isSetup()) return;
    elementCreator.clear();
    if (stage == 'd') {
        elementCreator.addSelectionElement(SelectionObjectElement(originSelection));
        elementCreator.addSelectionElement(SelectionElement(pointer, true));
    } else {
        elementCreator.addSelectionElement(SelectionElement(pointer));
    }
}
