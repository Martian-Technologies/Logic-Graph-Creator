#include <Qt>

#include "singlePlaceTool.h"
#include "gui/blockContainerView/renderer/renderer.h"

bool SinglePlaceTool::startPlaceBlock(const Event* event) {
    if (!blockContainer) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;
    switch (clicks[0]) {
    case 'n':
        clicks[0] = 'p';
        if (selectedBlock != NONE) blockContainer->tryInsertBlock(positionEvent->getPosition(), rotation, selectedBlock);
        return true;
    case 'p':
        return false;
    case 'r':
        if (clicks[1] == 'n') {
            clicks[1] = 'p';
            if (selectedBlock != NONE) blockContainer->tryInsertBlock(positionEvent->getPosition(), rotation, selectedBlock);
            return true;
        }
        return false;
    }
    return false;
}

bool SinglePlaceTool::stopPlaceBlock(const Event* event) {
    if (!blockContainer) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;
    // this logic is to allow holding right then left then releasing left and it to start deleting
    switch (clicks[0]) {
    case 'n':
        return false;
    case 'p':
        if (clicks[1] == 'r') {
            clicks[0] = 'r';
            clicks[1] = 'n';
        } else {
            clicks[0] = 'n';
        }
        return true;
    case 'r':
        if (clicks[1] == 'p') {
            clicks[1] = 'n';
            return true;
        }
        return false;
    }
    return false;
}

bool SinglePlaceTool::startDeleteBlocks(const Event* event) {
    if (!blockContainer) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;
    switch (clicks[0]) {
    case 'n':
        clicks[0] = 'r';
        blockContainer->tryRemoveBlock(positionEvent->getPosition());
        return true;
    case 'r':
        return false;
    case 'p':
        if (clicks[1] == 'n') {
            clicks[1] = 'r';
            blockContainer->tryRemoveBlock(positionEvent->getPosition());
            return true;
        }
        return false;
    }
    return false;
}

bool SinglePlaceTool::stopDeleteBlocks(const Event* event) {
    if (!blockContainer) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;
    // this logic is to allow holding left then right then releasing right and it to start placing
    switch (clicks[0]) {
    case 'n':
        return false;
    case 'r':
        if (clicks[1] == 'p') {
            clicks[0] = 'p';
            clicks[1] = 'n';
        } else {
            clicks[0] = 'n';
        }
        return true;
    case 'p':
        if (clicks[1] == 'r') {
            clicks[1] = 'n';
            return true;
        }
        return false;
    }
    return false;
}

bool SinglePlaceTool::pointerMove(const Event* event) {
    if (!blockContainer) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;
    bool returnVal = false; // used to make sure it updates the effect
    elementCreator.clear();
    elementCreator.addSelectionElement(positionEvent->getPosition());
    
    switch (clicks[0]) {
    case 'n':
        return returnVal;
    case 'r':
        if (clicks[1] == 'p') {
            if (selectedBlock != NONE) blockContainer->tryInsertBlock(positionEvent->getPosition(), rotation, selectedBlock);
            return selectedBlock != NONE;
        }
        blockContainer->tryRemoveBlock(positionEvent->getPosition());
        return true;
    case 'p':
        if (clicks[1] == 'r') {
            blockContainer->tryRemoveBlock(positionEvent->getPosition());
            return true;
        }
        if (selectedBlock != NONE) blockContainer->tryInsertBlock(positionEvent->getPosition(), rotation, selectedBlock);
        return selectedBlock != NONE;
    }
    return returnVal;
}

bool SinglePlaceTool::enterBlockView(const Event* event) {
    if (!blockContainer) return false;
    const PositionEvent* positionEvent = event->cast<PositionEvent>();
    if (!positionEvent) return false;
    elementCreator.clear();
    elementCreator.addSelectionElement(SelectionElement(positionEvent->getPosition()));
    return true;
}

bool SinglePlaceTool::exitBlockView(const Event* event) {
    if (!blockContainer) return false;
    elementCreator.clear();
    return true;
}
