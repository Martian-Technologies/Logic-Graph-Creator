#include <iostream>

#include <Qt>

#include "singlePlaceTool.h"
#include "../effects/cellSelectionEffect.h"

bool SinglePlaceTool::leftPress(const Position& pos) {
    if (!blockContainer) return false;
    switch (clicks[0]) {
    case 'n':
        clicks[0] = 'p';
        if (selectedBlock != NONE) blockContainer->tryInsertBlock(pos, rotation, selectedBlock);
        return true;
    case 'p':
        return false;
    case 'r':
        if (clicks[1] == 'n') {
            clicks[1] = 'p';
            if (selectedBlock != NONE) blockContainer->tryInsertBlock(pos, rotation, selectedBlock);
            return true;
        }
        return false;
    }
    return false;
}

bool SinglePlaceTool::leftRelease(const Position& pos) {
    if (!blockContainer) return false;
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

bool SinglePlaceTool::rightPress(const Position& pos) {
    if (!blockContainer) return false;
    switch (clicks[0]) {
    case 'n':
        clicks[0] = 'r';
        blockContainer->tryRemoveBlock(pos);
        return true;
    case 'r':
        return false;
    case 'p':
        if (clicks[1] == 'n') {
            clicks[1] = 'r';
            blockContainer->tryRemoveBlock(pos);
            return true;
        }
        return false;
    }
    return false;
}

bool SinglePlaceTool::rightRelease(const Position& pos) {
    if (!blockContainer) return false;
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

bool SinglePlaceTool::mouseMove(const Position& pos) {
    if (!blockContainer) return false;
    bool returnVal = false; // used to make sure it updates the effect

    if (effectDisplayer.hasEffect(0)) {
        effectDisplayer.getEffect<CellSelectionEffect>(0)->changeSelection(pos);
        returnVal = true;
    }
    
    switch (clicks[0]) {
    case 'n':
        return returnVal;
    case 'r':
        if (clicks[1] == 'p') {
            if (selectedBlock != NONE) blockContainer->tryInsertBlock(pos, rotation, selectedBlock);
            return selectedBlock != NONE;
        }
        blockContainer->tryRemoveBlock(pos);
        return true;
    case 'p':
        if (clicks[1] == 'r') {
            blockContainer->tryRemoveBlock(pos);
            return true;
        }
        if (selectedBlock != NONE) blockContainer->tryInsertBlock(pos, rotation, selectedBlock);
        return selectedBlock != NONE;
    }
    return returnVal;
}

bool SinglePlaceTool::enterBlockView(const Position& pos) {
    if (!blockContainer) return false;
    if (effectDisplayer.hasEffect(0)) return false;
    effectDisplayer.addEffect(CellSelectionEffect(0, 0, pos));
    return true;
}

bool SinglePlaceTool::exitBlockView(const Position& pos) {
    if (!blockContainer) return false;
    if (!effectDisplayer.hasEffect(0)) return false;
    effectDisplayer.removeEffect(0);
    return true;
}

bool SinglePlaceTool::keyPress(int keyId) {
    if (keyId == Qt::Key_Q) {
        rotation = rotate(rotation, false);
        return true;
    }
    if (keyId == Qt::Key_E) {
        rotation = rotate(rotation, true);
        return true;
    }
    return false;
}
