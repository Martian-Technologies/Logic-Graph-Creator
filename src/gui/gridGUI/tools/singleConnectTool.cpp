#include <iostream>

#include <Qt>

#include "singleConnectTool.h"
#include "../effects/cellSelectionEffect.h"

bool SingleConnectTool::leftPress(const Position& pos) {
    if (!blockContainer) return false;
    if (clicked) {
        if (!blockContainer->tryRemoveConnection(clickPosition, pos)) {
            blockContainer->tryCreateConnection(clickPosition, pos);
        }
        reset();
        return true;
    }
    clicked = true;
    clickPosition = pos;
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

bool SingleConnectTool::rightPress(const Position& pos) {
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

bool SingleConnectTool::mouseMove(const Position& pos) {
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

bool SingleConnectTool::enterBlockView(const Position& pos) {
    if (!blockContainer) return false;
    // if (effectDisplayer.hasEffect(0)) return false;
    // effectDisplayer.addEffect(CellSelectionEffect(0, 0, pos));
    return true;
}

bool SingleConnectTool::exitBlockView(const Position& pos) {
    if (!blockContainer) return false;
    // if (!effectDisplayer.hasEffect(0)) return false;
    // effectDisplayer.removeEffect(0);
    return true;
}
