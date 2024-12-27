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
    // switch (clicks[0]) {
    // case 'n':
    //     clicks[0] = 'p';
    //     if (selectedBlock != NONE) blockContainer->tryInsertBlock(pos, rotation, getBlockClass(selectedBlock));
    //     return true;
    // case 'p':
    //     return false;
    // case 'r':
    //     if (clicks[1] == 'n') {
    //         clicks[1] = 'p';
    //         if (selectedBlock != NONE) blockContainer->tryInsertBlock(pos, rotation, getBlockClass(selectedBlock));
    //         return true;
    //     }
    //     return false;
    // }
    return false;
}

bool SingleConnectTool::cancelConnection(const Event* event) {
    if (clicked) {
        reset();
        return true;
    }
    return false;
    // if (!blockContainer) return false;
    // switch (clicks[0]) {
    // case 'n':
    //     clicks[0] = 'r';
    //     blockContainer->tryRemoveBlock(pos);
    //     return true;
    // case 'r':
    //     return false;
    // case 'p':
    //     if (clicks[1] == 'n') {
    //         clicks[1] = 'r';
    //         blockContainer->tryRemoveBlock(pos);
    //         return true;
    //     }
    //     return false;
    // }
    // return false;
}

bool SingleConnectTool::pointerMove(const Event* event) {
    if (!blockContainer) return false;
    return false;
    // bool returnVal = false; // used to make sure it updates the effect

    // if (effectDisplayer.hasEffect(0)) {
    //     effectDisplayer.getEffect<CellSelectionEffect>(0)->changeSelection(pos);
    //     returnVal = true;
    // }

    // switch (clicks[0]) {
    // case 'n':
    //     return returnVal;
    // case 'r':
    //     if (clicks[1] == 'p') {
    //         if (selectedBlock != NONE) blockContainer->tryInsertBlock(pos, rotation, getBlockClass(selectedBlock));
    //         return selectedBlock != NONE;
    //     }
    //     blockContainer->tryRemoveBlock(pos);
    //     return true;
    // case 'p':
    //     if (clicks[1] == 'r') {
    //         blockContainer->tryRemoveBlock(pos);
    //         return true;
    //     }
    //     if (selectedBlock != NONE) blockContainer->tryInsertBlock(pos, rotation, getBlockClass(selectedBlock));
    //     return selectedBlock != NONE;
    // }
    // return returnVal;
}

bool SingleConnectTool::enterBlockView(const Event* event) {
    if (!blockContainer) return false;
    // if (effectDisplayer.hasEffect(0)) return false;
    // effectDisplayer.addEffect(CellSelectionEffect(0, 0, pos));
    return true;
}

bool SingleConnectTool::exitBlockView(const Event* event) {
    if (!blockContainer) return false;
    // if (!effectDisplayer.hasEffect(0)) return false;
    // effectDisplayer.removeEffect(0);
    return true;
}
